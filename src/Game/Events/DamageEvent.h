
#pragma once

#include "Physics/CMFwd.h"

namespace game
{
    struct DamageEvent
    {
        DamageEvent(const mono::IBodyPtr& body, int damage, float direction)
            : body(body)
            , damage(damage)
            , direction(direction)
        { }

        const mono::IBodyPtr body;
        const int damage;
        const float direction;
    };
}