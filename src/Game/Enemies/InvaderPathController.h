
#pragma once

#include "MonoPtrFwd.h"
#include "Enemy.h"
#include "Rendering/Color.h"

namespace game
{
    class InvaderPathController : public IEnemyController
    {
    public:

        InvaderPathController(const mono::IPathPtr& path, mono::EventHandler& event_handler);
        ~InvaderPathController();
        virtual void Initialize(Enemy* enemy);
        virtual void doUpdate(unsigned int delta);

    private:

        mono::IPathPtr m_path;
        mono::EventHandler& m_eventHandler;
        float m_currentPosition;
        int m_fireCount;
        int m_fireCooldown;

        math::Vector m_point;

        mono::IBodyPtr m_controlBody;
        mono::IConstraintPtr m_spring;
        std::unique_ptr<class IWeaponSystem> m_weapon;
        Enemy* m_enemy;
    };
}
