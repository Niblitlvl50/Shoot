
#pragma once

#include "IDamageModifier.h"

namespace game
{
    class DamageReductionModifier : public IDamageModifier
    {
    public:
        DamageReductionModifier(const char* name_identifier, float multiplier);
        uint32_t Id() const override;
        DamageDetails ModifyDamage(const DamageDetails& damage_details) const override;
    private:
        const uint32_t m_id;
        const float m_multiplier;
    };

    class DodgeChanceModifier : public IDamageModifier
    {
    public:
        DodgeChanceModifier(const char* name_identifier, float chance);
        uint32_t Id() const override;
        FilterResult FilterDamage(uint32_t entity_id, uint32_t who_did_damage, uint32_t weapon_id, int damage) const override;
    private:
        const uint32_t m_id;
        const float m_chance;
    };
}
