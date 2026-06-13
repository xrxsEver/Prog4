#include "LoggingSoundSystem.h"
#include "AudioLogger.h"
#include <iostream>
#include <sstream>

namespace dae
{
    LoggingSoundSystem::LoggingSoundSystem(std::unique_ptr<SoundSystem>&& ss)
        : _real_ss(std::move(ss))
    {
    }

    void LoggingSoundSystem::play(const sound_id id, const float volume)
    {
        std::stringstream ss;
        ss << "Playing sound " << id << " with volume " << volume;
        AudioLogger::Log(ss.str());
        std::cout << ss.str() << std::endl;
        _real_ss->play(id, volume);
    }

    void LoggingSoundSystem::play(const std::string& path, const float volume)
    {
        std::stringstream ss;
        ss << "Playing sound " << path << " with volume " << volume;
        AudioLogger::Log(ss.str());
        std::cout << ss.str() << std::endl;
        _real_ss->play(path, volume);
    }

    void LoggingSoundSystem::play_music(const std::string& path, const float volume, bool loop)
    {
        std::stringstream ss;
        ss << "Playing music " << path << " with volume " << volume << " (loop: " << (loop ? "yes" : "no") << ")";
        AudioLogger::Log(ss.str());
        std::cout << ss.str() << std::endl;
        _real_ss->play_music(path, volume, loop);
    }

    void LoggingSoundSystem::stop_music()
    {
        AudioLogger::Log("Stopping music");
        std::cout << "Stopping music" << std::endl;
        _real_ss->stop_music();
    }

    void LoggingSoundSystem::pause_music()
    {
        AudioLogger::Log("Pausing music");
        std::cout << "Pausing music" << std::endl;
        _real_ss->pause_music();
    }

    void LoggingSoundSystem::resume_music()
    {
        AudioLogger::Log("Resuming music");
        std::cout << "Resuming music" << std::endl;
        _real_ss->resume_music();
    }

    bool LoggingSoundSystem::is_music_playing() const
    {
        return _real_ss->is_music_playing();
    }

    void LoggingSoundSystem::set_muted(bool muted)
    {
        AudioLogger::Log(muted ? "Muting audio" : "Unmuting audio");
        std::cout << (muted ? "Muting audio" : "Unmuting audio") << std::endl;
        _real_ss->set_muted(muted);
    }

    bool LoggingSoundSystem::is_muted() const
    {
        return _real_ss->is_muted();
    }

    std::vector<std::string> LoggingSoundSystem::get_audio_devices() const
    {
        return _real_ss->get_audio_devices();
    }

    void LoggingSoundSystem::set_audio_device(int index)
    {
        _real_ss->set_audio_device(index);
    }

    int LoggingSoundSystem::get_current_device_index() const
    {
        return _real_ss->get_current_device_index();
    }
}
