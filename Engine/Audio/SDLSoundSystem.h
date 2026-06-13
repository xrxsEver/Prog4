#ifndef SDLSOUNDSYSTEM_H
#define SDLSOUNDSYSTEM_H

#include "SoundSystem.h"
#include <memory>

namespace dae
{
    class SDLSoundSystem final : public SoundSystem
    {
    public:
        SDLSoundSystem(const std::string& dataPath = "");
        ~SDLSoundSystem() override;

        SDLSoundSystem(const SDLSoundSystem& other) = delete;
        SDLSoundSystem(SDLSoundSystem&& other) = delete;
        SDLSoundSystem& operator=(const SDLSoundSystem& other) = delete;
        SDLSoundSystem& operator=(SDLSoundSystem&& other) = delete;

        void play(const sound_id id, const float volume) override;
        void play(const std::string& path, const float volume) override;
        void play_music(const std::string& path, const float volume, bool loop) override;
        void stop_music() override;
        void pause_music() override;
        void resume_music() override;

        bool is_music_playing() const override;
        void set_muted(bool muted) override;
        bool is_muted() const override;

        std::vector<std::string> get_audio_devices() const override;
        void set_audio_device(int index) override;
        int get_current_device_index() const override;

    private:
        class SDLSoundSystemImpl;
        std::unique_ptr<SDLSoundSystemImpl> _pImpl;
    };
}

#endif // SDLSOUNDSYSTEM_H
