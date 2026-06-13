#ifndef LOGGINGSOUNDSYSTEM_H
#define LOGGINGSOUNDSYSTEM_H

#include "SoundSystem.h"
#include <memory>

namespace dae
{
    class LoggingSoundSystem final : public SoundSystem
    {
    public:
        explicit LoggingSoundSystem(std::unique_ptr<SoundSystem>&& ss);
        ~LoggingSoundSystem() override = default;

        LoggingSoundSystem(const LoggingSoundSystem& other) = delete;
        LoggingSoundSystem(LoggingSoundSystem&& other) = delete;
        LoggingSoundSystem& operator=(const LoggingSoundSystem& other) = delete;
        LoggingSoundSystem& operator=(LoggingSoundSystem&& other) = delete;

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
        std::unique_ptr<SoundSystem> _real_ss;
    };
}

#endif // LOGGINGSOUNDSYSTEM_H
