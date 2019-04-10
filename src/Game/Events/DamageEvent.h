
#pragma once

#include "Physics/PhysicsFwd.h"

namespace game
{
    struct DamageEvent
    {
        DamageEvent(const mono::IBody* body, int damage, float direction)
            : body(body)
            , damage(damage)
            , direction(direction)
        { }

        const mono::IBody* body;
        const int damage;
        const float direction;
    };
}