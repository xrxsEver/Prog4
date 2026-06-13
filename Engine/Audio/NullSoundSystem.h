#ifndef NULLSOUNDSYSTEM_H
#define NULLSOUNDSYSTEM_H

#include "SoundSystem.h"

namespace dae
{
    class NullSoundSystem final : public SoundSystem
    {
    public:
        void play(const sound_id, const float) override {}
        void play(const std::string&, const float) override {}
        void play_music(const std::string&, const float, bool) override {}
        void stop_music() override {}
        void pause_music() override {}
        void resume_music() override {}

        bool is_music_playing() const override { return false; }
        void set_muted(bool) override {}
        bool is_muted() const override { return false; }

        std::vector<std::string> get_audio_devices() const override { return {}; }
        void set_audio_device(int) override {}
        int get_current_device_index() const override { return -1; }
    };
}

#endif // NULLSOUNDSYSTEM_H
