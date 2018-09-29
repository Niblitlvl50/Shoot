
#pragma once

#include "Enemy.h"
#include "Weapons/IWeaponSystem.h"

namespace game
{
    class CacoDemonController : public IEnemyController
    {
    public:

        CacoDemonController(mono::EventHandler& event_handler);
        ~CacoDemonController();

        void Initialize(Enemy* enemy) override;
        void doUpdate(unsigned int delta) override;

    private:

        mono::EventHandler& m_event_handler;

        Enemy* m_enemy;
        std::unique_ptr<IWeaponSystem> m_weapon;
        std::unique_ptr<class TrackingBehaviour> m_tracking_behaviour;
    };
}
