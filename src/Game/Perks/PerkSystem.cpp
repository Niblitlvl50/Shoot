
#include "PerkSystem.h"
#include "PerkModifiers.h"
#include "Serialize.h"
#include "System/File.h"
#include "System/System.h"
#include "Util/Random.h"

#include "Player/PlayerInfo.h"
#include "Weapons/WeaponSystem.h"
#include "Weapons/IWeaponModifier.h"
#include "Weapons/Modifiers/BulletBehaviourModifiers.h"
#include "Weapons/Modifiers/DamageModifier.h"
#include "Weapons/WeaponConfiguration.h"

#include "nlohmann/json.hpp"

using namespace game;

PerkSystem::PerkSystem(WeaponSystem* weapon_system)
    : m_weapon_system(weapon_system)
{
    const std::vector<byte> file_data = file::FileReadAll("res/configs/perks_config.json");
    const nlohmann::json& json = nlohmann::json::parse(file_data);
    m_perk_setup = json["setup"];
    m_perk_definitions = json["perks"];
}

const char* PerkSystem::Name() const
{
    return "PerkSystem";
}

void PerkSystem::Update(const mono::UpdateContext& update_context)
{
    m_current_reroll_time += update_context.delta_s;
}

float PerkSystem::GetCurrentRerollFraction() const
{
    return std::clamp(m_current_reroll_time / m_perk_setup.reroll_duration, 0.0f, 1.0f);
}

const PerkDefinition& PerkSystem::GetCurrentPlayerPerk() const
{
    return m_perk_definitions[m_current_player_perk_id];
}

const PerkDefinition& PerkSystem::GetCurrentEnemyPerk() const
{
    return m_perk_definitions[m_current_enemy_perk_id];
}

void PerkSystem::RollForNewPlayerPerk()
{
    RemoveCurrentPlayerPerk();
    m_current_player_perk_id = RerollPerkAvoidId(m_current_player_perk_id);
    ApplyPerkToPlayers(GetCurrentPlayerPerk());
    System::Log("PerkSystem|Player perk: %s", GetCurrentPlayerPerk().name.c_str());
}

void PerkSystem::RollForNewEnemyPerk()
{
    m_current_enemy_perk_id = RerollPerkAvoidId(m_current_enemy_perk_id);
    m_enemy_modifier.reset(CreateModifierForPerk(GetCurrentEnemyPerk().type));
    System::Log("PerkSystem|Enemy perk: %s", GetCurrentEnemyPerk().name.c_str());
}

IWeaponModifier* PerkSystem::GetCurrentEnemyModifier() const
{
    return m_enemy_modifier.get();
}

uint32_t PerkSystem::RerollPerkAvoidId(uint32_t perk_id_to_avoid)
{
    uint32_t new_perk_id = perk_id_to_avoid;
    while(new_perk_id == perk_id_to_avoid)
        new_perk_id = mono::Random(0, m_perk_definitions.size() - 1);

    return new_perk_id;
}

void PerkSystem::ApplyPerkToPlayers(const PerkDefinition& perk)
{
    m_player_modifier.reset(CreateModifierForPerk(perk.type));
    if(!m_player_modifier)
        return;

    const PlayerArray players = GetSpawnedPlayers();
    for(const PlayerInfo* player : players)
    {
        if(!player)
            continue;
        const int slot_id = m_weapon_system->AddModifierForId(player->entity_id, m_player_modifier.get());
        m_player_modifier_slots[player->entity_id] = slot_id;
    }
}

void PerkSystem::RemoveCurrentPlayerPerk()
{
    for(const auto& pair : m_player_modifier_slots)
        m_weapon_system->RemoveModifierForEntity(pair.first, pair.second);
    m_player_modifier_slots.clear();
    m_player_modifier.reset();
}


IWeaponModifier* PerkSystem::CreateModifierForPerk(PerkType type)
{
    switch(type)
    {
    case PerkType::IncreasedDamage:
        return new DamageModifier("perk_increased_damage", 1.5f);
    case PerkType::IncreasedFireRate:
        return new WeaponFireRateModifier("perk_increased_firerate", 1.5f);
    case PerkType::IncreasedBulletVelocity:
        return new BulletVelocityModifier("perk_increased_velocity", 1.5f);
    case PerkType::IncreasedBulletSpread:
        return new BulletSpreadModifier("perk_increased_spread", 2.0f);
    case PerkType::IncreasedBulletRange:
        return new BulletRangeModifier("perk_increased_range", 1.5f);
    case PerkType::IncreasedMagazineSize:
        return new MagazineSizeModifier("perk_increased_magazine", 10);
    case PerkType::InfiniteAmmo:
        return new InfiniteAmmoModifier();
    case PerkType::PiercingBullets:
        return new BulletBehaviourModifier("perk_piercing", BulletCollisionFlag::PASS_THROUGH);
    case PerkType::ExplosiveBullets:
        return new BulletBehaviourModifier("perk_explosive", BulletCollisionFlag::EXPLODES);
    case PerkType::HomingBullets:
        return new BulletMovementModifier("perk_homing", BulletMovementFlag::HOMING);
    case PerkType::RicochetBullets:
        return new BulletBehaviourModifier("perk_ricochet", BulletCollisionFlag::BOUNCE);
    case PerkType::VampiricBullets:
        return new BulletBehaviourModifier("perk_vampiric", BulletCollisionFlag::VAMPERIC);
    default:
        return nullptr;
    }
}
