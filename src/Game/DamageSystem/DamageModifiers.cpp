
#include "DamageModifiers.h"
#include "System/Hash.h"
#include "Util/Random.h"

using namespace game;

DamageReductionModifier::DamageReductionModifier(const char* name_identifier, float multiplier)
    : m_id(hash::Hash(name_identifier))
    , m_multiplier(multiplier)
{ }

uint32_t DamageReductionModifier::Id() const
{
    return m_id;
}

DamageDetails DamageReductionModifier::ModifyDamage(const DamageDetails& damage_details) const
{
    DamageDetails modified = damage_details;
    modified.damage = static_cast<int>(modified.damage * m_multiplier);
    return modified;
}


DodgeChanceModifier::DodgeChanceModifier(const char* name_identifier, float chance)
    : m_id(hash::Hash(name_identifier))
    , m_chance(chance)
{ }

uint32_t DodgeChanceModifier::Id() const
{
    return m_id;
}

FilterResult DodgeChanceModifier::FilterDamage(uint32_t entity_id, uint32_t who_did_damage, uint32_t weapon_id, int damage) const
{
    if(mono::Chance(m_chance))
        return FilterResult::FILTER_OUT;
    return FilterResult::APPLY_DAMAGE;
}
