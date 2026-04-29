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

        virtual std::vector<std::string> get_audio_devices() const = 0;
        virtual void set_audio_device(int index) = 0;
        virtual int get_current_device_index() const = 0;
    };
}

#endif // SOUNDSYSTEM_H
