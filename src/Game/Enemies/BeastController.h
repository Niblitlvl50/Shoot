
#pragma once

#include "Enemy.h"

namespace game
{
    class BeastController : public IEnemyController
    {
    public:

        BeastController(mono::EventHandler& event_handler);

        void Initialize(class Enemy* enemy) override;
        void doUpdate(unsigned int delta) override;
    };
}
