#ifndef SOUNDSYSTEM_H
#define SOUNDSYSTEM_H

#include <string>
#include <vector>

namespace dae
{
    using sound_id = unsigned short;

    class SoundSystem
    {
    public:
        virtual ~SoundSystem() = default;
        virtual void play(const sound_id id, const float volume) = 0;
        virtual void play(const std::string& path, const float volume) = 0;
        virtual void play_music(const std::string& path, const float volume, bool loop = true) = 0;
        virtual void stop_music() = 0;
        virtual void pause_music() = 0;
        virtual void resume_music() = 0;

        // True while a music track is actively sounding (not stopped, not paused). Lets game code
        // chain tracks — e.g. start the looping theme the moment a one-shot jingle finishes.
        virtual bool is_music_playing() const = 0;

        // Master mute: silences everything (music + sfx) without forgetting what's playing.
        virtual void set_muted(bool muted) = 0;
        virtual bool is_muted() const = 0;

        virtual std::vector<std::string> get_audio_devices() const = 0;
        virtual void set_audio_device(int index) = 0;
        virtual int get_current_device_index() const = 0;
    };
}

#endif // SOUNDSYSTEM_H
