#include "SDLSoundSystem.h"
#include "AudioLogger.h"
#include <SDL3_mixer/SDL_mixer.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <vector>
#include <string>
#include <unordered_map>
#include <iostream>
#include <filesystem>

// For Emscripten, we can't use std::thread without special build flags.
// So, we'll process audio commands synchronously on the main thread.
#ifdef __EMSCRIPTEN__
#define NO_THREADING
#endif

namespace dae
{
    namespace fs = std::filesystem;
    enum class AudioCommandType
    {
        PlaySound,
        PlaySoundPath,
        PlayMusic,
        StopMusic,
        PauseMusic,
        ResumeMusic,
        ChangeDevice
    };

    struct AudioCommand
    {
        AudioCommandType type;
        sound_id id;
        float volume;
        std::string path;
        bool loop;
        int deviceIndex;
    };

    class SDLSoundSystem::SDLSoundSystemImpl
    {
    public:
        SDLSoundSystemImpl(const std::string& dataPath)
            : _dataPath(dataPath)
        {
            if (!SDL_InitSubSystem(SDL_INIT_AUDIO))
            {
                std::string error = "SDL_InitSubSystem(SDL_INIT_AUDIO) failed! Error: ";
                error += SDL_GetError();
                std::cerr << error << std::endl;
                AudioLogger::Log(error);
                return;
            }

            if (!MIX_Init())
            {
                std::string error = "MIX_Init failed! SDL_mixer Error: ";
                error += SDL_GetError();
                std::cerr << error << std::endl;
                AudioLogger::Log(error);
                return;
            }

            RefreshDevices();
            // Default to SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK
            InitMixer(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK);

#ifndef NO_THREADING
            _thread = std::thread(&SDLSoundSystemImpl::ProcessQueue, this);
#endif
        }

        ~SDLSoundSystemImpl()
        {
#ifndef NO_THREADING
            _stopThread = true;
            _cv.notify_all();
            if (_thread.joinable())
                _thread.join();
#endif
            CleanupMixer();
            MIX_Quit();
            SDL_QuitSubSystem(SDL_INIT_AUDIO);
        }

        void AddCommand(AudioCommand&& cmd)
        {
#ifdef NO_THREADING
            ExecuteCommand(cmd);
#else
            std::lock_guard<std::mutex> lock(_mutex);
            _queue.push(std::move(cmd));
            _cv.notify_one();
#endif
        }

        std::vector<std::string> GetAudioDevices()
        {
#ifndef NO_THREADING
            std::lock_guard<std::mutex> lock(_mutex);
#endif
            RefreshDevices();
            return _devices;
        }

        void SetAudioDevice(int index)
        {
            AddCommand({AudioCommandType::ChangeDevice, 0, 0, "", false, index});
        }

        int GetCurrentDeviceIndex() const
        {
            return _currentDeviceIndex;
        }

        // Queried every frame by the level music hand-off; SDL_mixer's MIX_* queries are documented
        // safe to call from any thread, so this reads the music track directly off the main thread.
        bool IsMusicPlaying() const
        {
            return _musicTrack && MIX_TrackPlaying(_musicTrack);
        }

        void SetMuted(bool muted)
        {
            _muted = muted;
            if (_mixer)
            {
                // Master gain of 0 silences the whole mix; 1 leaves it untouched.
                MIX_SetMixerGain(_mixer, muted ? 0.0f : 1.0f);
            }
        }

        bool IsMuted() const
        {
            return _muted;
        }

    private:
        void RefreshDevices()
        {
            int count = 0;
            SDL_AudioDeviceID* devices = SDL_GetAudioPlaybackDevices(&count);
            _devices.clear();
            _deviceIDs.clear();
            
            // Add Default Device
            _devices.push_back("Default Device");
            _deviceIDs.push_back(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK);

            if (devices)
            {
                for (int i = 0; i < count; ++i)
                {
                    const char* name = SDL_GetAudioDeviceName(devices[i]);
                    _devices.push_back(name ? name : "Unknown Device");
                    _deviceIDs.push_back(devices[i]);
                }
                SDL_free(devices);
            }
        }

        void InitMixer(SDL_AudioDeviceID devid)
        {
            _mixer = MIX_CreateMixerDevice(devid, nullptr);
            if (!_mixer)
            {
                std::string error = "MIX_CreateMixerDevice failed! SDL_mixer Error: ";
                error += SDL_GetError();
                std::cerr << error << std::endl;
                AudioLogger::Log(error);
                return;
            }

            // Create SFX tracks
            for (int i = 0; i < 8; ++i)
            {
                _sfxTracks.push_back(MIX_CreateTrack(_mixer));
            }
            // Dedicated music track
            _musicTrack = MIX_CreateTrack(_mixer);

            // A fresh mixer defaults to full gain, so re-apply mute after a device switch.
            MIX_SetMixerGain(_mixer, _muted ? 0.0f : 1.0f);

            AudioLogger::Log("Audio mixer initialized successfully.");
        }

        void CleanupMixer()
        {
            for (auto& pair : _sounds)
            {
                MIX_DestroyAudio(pair.second);
            }
            _sounds.clear();

            for (auto& pair : _pathSounds)
            {
                MIX_DestroyAudio(pair.second);
            }
            _pathSounds.clear();
            
            if (_music)
            {
                MIX_DestroyAudio(_music);
                _music = nullptr;
            }
            
            _sfxTracks.clear();
            _musicTrack = nullptr;

            if (_mixer)
            {
                MIX_DestroyMixer(_mixer);
                _mixer = nullptr;
            }
        }

    private:
#ifndef NO_THREADING
        void ProcessQueue()
        {
            while (!_stopThread)
            {
                AudioCommand cmd;
                {
                    std::unique_lock<std::mutex> lock(_mutex);
                    _cv.wait(lock, [this] { return !_queue.empty() || _stopThread; });

                    if (_stopThread && _queue.empty())
                        break;

                    if (!_queue.empty())
                    {
                        cmd = std::move(_queue.front());
                        _queue.pop();
                    }
                    else
                    {
                        continue;
                    }
                }

                ExecuteCommand(cmd);
            }
        }
#endif

        void ExecuteCommand(const AudioCommand& cmd)
        {
            switch (cmd.type)
            {
            case AudioCommandType::PlaySound:
                PlaySound(cmd.id, cmd.volume);
                break;
            case AudioCommandType::PlaySoundPath:
                PlaySound(cmd.path, cmd.volume);
                break;
            case AudioCommandType::PlayMusic:
                PlayMusic(cmd.path, cmd.volume, cmd.loop);
                break;
            case AudioCommandType::StopMusic:
                if (_musicTrack) MIX_StopTrack(_musicTrack, 0);
                _currentMusicPath = "";
                break;
            case AudioCommandType::PauseMusic:
                if (_musicTrack) MIX_PauseTrack(_musicTrack);
                _isMusicPaused = true;
                break;
            case AudioCommandType::ResumeMusic:
                if (_musicTrack) MIX_ResumeTrack(_musicTrack);
                _isMusicPaused = false;
                break;
            case AudioCommandType::ChangeDevice:
                ChangeDevice(cmd.deviceIndex);
                break;
            }
        }

        void ChangeDevice(int index)
        {
#ifndef NO_THREADING
            std::lock_guard<std::mutex> lock(_mutex);
#endif
            if (index < 0 || index >= (int)_deviceIDs.size())
                return;

            if (index == _currentDeviceIndex)
                return;

            AudioLogger::Log("Changing audio device to: " + _devices[index]);
            
            Sint64 musicFrame = 0;
            if (_musicTrack)
            {
                musicFrame = MIX_GetTrackPlaybackPosition(_musicTrack);
            }

            CleanupMixer();
            InitMixer(_deviceIDs[index]);
            _currentDeviceIndex = index;

            if (!_currentMusicPath.empty())
            {
                bool wasPaused = _isMusicPaused;
                PlayMusic(_currentMusicPath, _currentMusicVolume, _currentMusicLoop);
                _isMusicPaused = wasPaused;
                if (_musicTrack)
                {
                    if (musicFrame > 0)
                    {
                        MIX_SetTrackPlaybackPosition(_musicTrack, musicFrame);
                    }
                    if (_isMusicPaused)
                    {
                        MIX_PauseTrack(_musicTrack);
                    }
                }
            }
        }

        void PlaySound(const sound_id id, const float volume)
        {
            MIX_Audio* audio = GetAudio(id);
            if (audio)
            {
                PlayAudioOnNextTrack(audio, volume);
            }
        }

        void PlaySound(const std::string& path, const float volume)
        {
            if (!_mixer) return;

            auto it = _pathSounds.find(path);
            if (it != _pathSounds.end())
            {
                PlayAudioOnNextTrack(it->second, volume);
                return;
            }

            fs::path soundPath = path;
            if (!fs::exists(soundPath) && !_dataPath.empty())
            {
                soundPath = fs::path(_dataPath) / path;
            }

            MIX_Audio* audio = MIX_LoadAudio(_mixer, soundPath.string().c_str(), true);
            if (audio)
            {
                _pathSounds[path] = audio;
                PlayAudioOnNextTrack(audio, volume);
            }
            else
            {
                std::string error = "Failed to load sound " + soundPath.string() + "! Error: ";
                error += SDL_GetError();
                std::cerr << error << std::endl;
                AudioLogger::Log(error);
            }
        }

        void PlayAudioOnNextTrack(MIX_Audio* audio, float volume)
        {
            if (!audio || _sfxTracks.empty()) return;

            MIX_Track* track = _sfxTracks[_nextTrackIndex];
            _nextTrackIndex = (_nextTrackIndex + 1) % (int)_sfxTracks.size();

            MIX_SetTrackGain(track, volume);
            MIX_SetTrackAudio(track, audio);

            SDL_PropertiesID props = SDL_CreateProperties();
            SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER, 0);
            if (!MIX_PlayTrack(track, props))
            {
                std::string error = "MIX_PlayTrack failed! Error: ";
                error += SDL_GetError();
                AudioLogger::Log(error);
            }
            SDL_DestroyProperties(props);
        }

        void PlayMusic(const std::string& path, const float volume, bool loop)
        {
            if (!_mixer) return;

            _currentMusicPath = path;
            _currentMusicVolume = volume;
            _currentMusicLoop = loop;
            _isMusicPaused = false;

            if (_music)
            {
                MIX_DestroyAudio(_music);
                _music = nullptr;
            }

            fs::path musicPath = path;
            if (!fs::exists(musicPath) && !_dataPath.empty())
            {
                musicPath = fs::path(_dataPath) / path;
            }

            _music = MIX_LoadAudio(_mixer, musicPath.string().c_str(), false);
            if (_music && _musicTrack)
            {
                MIX_SetTrackGain(_musicTrack, volume);
                MIX_SetTrackAudio(_musicTrack, _music);
                
                SDL_PropertiesID props = SDL_CreateProperties();
                SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER, loop ? -1 : 0);
                MIX_PlayTrack(_musicTrack, props);
                SDL_DestroyProperties(props);
            }
            else
            {
                std::string error = "Failed to load music " + musicPath.string() + "! Error: ";
                error += SDL_GetError();
                std::cerr << error << std::endl;
                AudioLogger::Log(error);
            }
        }

        MIX_Audio* GetAudio(const sound_id id)
        {
            auto it = _sounds.find(id);
            if (it != _sounds.end())
                return it->second;

            // Try .wav then .mp3
            fs::path basePath = _dataPath.empty() ? fs::path("Data/Sounds/") : fs::path(_dataPath) / "Sounds/";
            fs::path path = basePath / (std::to_string(id) + ".wav");
            
            MIX_Audio* audio = MIX_LoadAudio(_mixer, path.string().c_str(), true);
            if (!audio)
            {
                path = basePath / (std::to_string(id) + ".mp3");
                audio = MIX_LoadAudio(_mixer, path.string().c_str(), true);
            }

            if (audio)
            {
                _sounds[id] = audio;
            }
            else
            {
                std::string error = "Failed to load sound " + std::to_string(id) + " (.wav/.mp3)! Error: ";
                error += SDL_GetError();
                std::cerr << error << std::endl;
                AudioLogger::Log(error);
            }
            return audio;
        }

#ifndef NO_THREADING
        std::thread _thread;
        std::mutex _mutex;
        std::condition_variable _cv;
        std::queue<AudioCommand> _queue;
        std::atomic<bool> _stopThread{false};
#endif

        MIX_Mixer* _mixer = nullptr;
        std::vector<MIX_Track*> _sfxTracks;
        MIX_Track* _musicTrack = nullptr;
        int _nextTrackIndex = 0;
        std::string _dataPath;
        
        std::vector<std::string> _devices;
        std::vector<SDL_AudioDeviceID> _deviceIDs;
        int _currentDeviceIndex = 0;

        std::unordered_map<sound_id, MIX_Audio*> _sounds;
        std::unordered_map<std::string, MIX_Audio*> _pathSounds;
        MIX_Audio* _music = nullptr;

        std::string _currentMusicPath;
        float _currentMusicVolume{ 0.0f };
        bool _currentMusicLoop{ false };
        bool _isMusicPaused{ false };
        std::atomic<bool> _muted{ false };
    };

    SDLSoundSystem::SDLSoundSystem(const std::string& dataPath)
        : _pImpl(std::make_unique<SDLSoundSystemImpl>(dataPath))
    {
    }

    SDLSoundSystem::~SDLSoundSystem() = default;

    void SDLSoundSystem::play(const sound_id id, const float volume)
    {
        _pImpl->AddCommand({AudioCommandType::PlaySound, id, volume, "", false, 0});
    }

    void SDLSoundSystem::play(const std::string& path, const float volume)
    {
        _pImpl->AddCommand({AudioCommandType::PlaySoundPath, 0, volume, path, false, 0});
    }

    void SDLSoundSystem::play_music(const std::string& path, const float volume, bool loop)
    {
        _pImpl->AddCommand({AudioCommandType::PlayMusic, 0, volume, path, loop, 0});
    }

    void SDLSoundSystem::stop_music()
    {
        _pImpl->AddCommand({AudioCommandType::StopMusic, 0, 0, "", false, 0});
    }

    void SDLSoundSystem::pause_music()
    {
        _pImpl->AddCommand({AudioCommandType::PauseMusic, 0, 0, "", false, 0});
    }

    void SDLSoundSystem::resume_music()
    {
        _pImpl->AddCommand({AudioCommandType::ResumeMusic, 0, 0, "", false, 0});
    }

    bool SDLSoundSystem::is_music_playing() const
    {
        return _pImpl->IsMusicPlaying();
    }

    void SDLSoundSystem::set_muted(bool muted)
    {
        _pImpl->SetMuted(muted);
    }

    bool SDLSoundSystem::is_muted() const
    {
        return _pImpl->IsMuted();
    }

    std::vector<std::string> SDLSoundSystem::get_audio_devices() const
    {
        return _pImpl->GetAudioDevices();
    }

    void SDLSoundSystem::set_audio_device(int index)
    {
        _pImpl->SetAudioDevice(index);
    }

    int SDLSoundSystem::get_current_device_index() const
    {
        return _pImpl->GetCurrentDeviceIndex();
    }
}
