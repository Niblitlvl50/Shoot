
#pragma once

#include "DamageSystemTypes.h"
#include <cstdint>

namespace game
{
    class IDamageModifier
    {
    public:

        virtual ~IDamageModifier() = default;

        virtual uint32_t Id() const { return 0; }
        virtual FilterResult FilterDamage(uint32_t entity_id, uint32_t who_did_damage, uint32_t weapon_id, int damage) const { return FilterResult::APPLY_DAMAGE; }
        virtual DamageDetails ModifyDamage(const DamageDetails& damage_details) const { return damage_details; }
    };
}
