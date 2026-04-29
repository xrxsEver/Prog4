#include "ServiceLocator.h"
#include "NullSoundSystem.h"

namespace dae
{
    std::unique_ptr<SoundSystem> ServiceLocator::_ss_instance = std::make_unique<NullSoundSystem>();

    SoundSystem& ServiceLocator::get_sound_system()
    {
        return *_ss_instance;
    }

    void ServiceLocator::register_sound_system(std::unique_ptr<SoundSystem>&& ss)
    {
        _ss_instance = (ss == nullptr) ? std::make_unique<NullSoundSystem>() : std::move(ss);
    }
}
