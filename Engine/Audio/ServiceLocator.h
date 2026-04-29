#ifndef SERVICELOCATOR_H
#define SERVICELOCATOR_H

#include <memory>
#include "SoundSystem.h"

namespace dae
{
    class ServiceLocator final
    {
    public:
        static SoundSystem& get_sound_system();
        static void register_sound_system(std::unique_ptr<SoundSystem>&& ss);

    private:
        static std::unique_ptr<SoundSystem> _ss_instance;
    };
}

#endif // SERVICELOCATOR_H
