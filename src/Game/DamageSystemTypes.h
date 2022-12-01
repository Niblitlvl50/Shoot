
#pragma once

#include <cstdint>
#include <functional>

#define ENUM_BIT(n) (1 << (n))

namespace game
{
    enum DamageType
    {
        DESTROYED   = ENUM_BIT(0),
        DAMAGED     = ENUM_BIT(1),
        DT_ALL      = DESTROYED | DAMAGED,
    };

    using DamageCallback = std::function<void (uint32_t id, int damage, uint32_t who_did_damage, DamageType type)>;
}
