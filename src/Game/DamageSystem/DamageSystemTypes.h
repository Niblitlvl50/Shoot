
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
    using DamageCallback = std::function<void (uint32_t damaged_entity_id, uint32_t who_did_damage, uint32_t weapon_identifier, int damage, DamageType type)>;

    enum class FilterResult
    {
        APPLY_DAMAGE,
        FILTER_OUT
    };
    using DamageFilter = std::function<FilterResult (uint32_t damaged_entity_id, uint32_t who_did_damage, uint32_t weapon_identifier, int damage)>;
}
