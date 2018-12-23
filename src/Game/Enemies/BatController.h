
#pragma once

#include "Enemy.h"

namespace game
{
    class BatController : public IEnemyController
    {
    public:

        BatController(mono::EventHandler& event_handler);
        void Initialize(class Enemy* enemy) override;
        void doUpdate(unsigned int delta) override;

    private:

        Enemy* m_enemy;
    };
}
