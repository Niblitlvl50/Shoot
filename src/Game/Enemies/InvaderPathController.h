
#pragma once

#include "MonoPtrFwd.h"
#include "Enemy.h"

namespace game
{
    class InvaderPathController : public IEnemyController
    {
    public:

        InvaderPathController(mono::IPathPtr& path, mono::EventHandler& event_handler);
        ~InvaderPathController();

        void Initialize(Enemy* enemy) override;
        void doUpdate(unsigned int delta) override;

    private:

        const mono::IPathPtr m_path;
        mono::EventHandler& m_event_handler;
        int m_fire_count;
        int m_fire_cooldown;

        Enemy* m_enemy;
        std::unique_ptr<class PathBehaviour> m_path_behaviour;
        std::unique_ptr<class IWeaponSystem> m_weapon;
    };
}
