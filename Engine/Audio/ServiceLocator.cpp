#include "ServiceLocator.h"
#include "NullSoundSystem.h"
#include "CollisionGrid.h"

namespace dae
{
    std::unique_ptr<SoundSystem> ServiceLocator::_ss_instance = std::make_unique<NullSoundSystem>();
    std::unique_ptr<CollisionGrid> ServiceLocator::_grid_instance = std::make_unique<CollisionGrid>();

    SoundSystem& ServiceLocator::get_sound_system()
    {
        return *_ss_instance;
    }

    void ServiceLocator::register_sound_system(std::unique_ptr<SoundSystem>&& ss)
    {
        _ss_instance = (ss == nullptr) ? std::make_unique<NullSoundSystem>() : std::move(ss);
    }

    CollisionGrid& ServiceLocator::get_collision_grid()
    {
        return *_grid_instance;
    }

    void ServiceLocator::register_collision_grid(std::unique_ptr<CollisionGrid>&& grid)
    {
        _grid_instance = (grid == nullptr) ? std::make_unique<CollisionGrid>() : std::move(grid);
    }
}
