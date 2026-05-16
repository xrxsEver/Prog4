#ifndef SERVICELOCATOR_H
#define SERVICELOCATOR_H

#include <memory>
#include "SoundSystem.h"
#include "CollisionGrid.h"

namespace dae
{
    class ServiceLocator final
    {
    public:
        static SoundSystem& get_sound_system();
        static void register_sound_system(std::unique_ptr<SoundSystem>&& ss);

        static CollisionGrid& get_collision_grid();
        static void register_collision_grid(std::unique_ptr<CollisionGrid>&& grid);

    private:
        static std::unique_ptr<SoundSystem> _ss_instance;
        static std::unique_ptr<CollisionGrid> _grid_instance;
    };
}

#endif // SERVICELOCATOR_H
