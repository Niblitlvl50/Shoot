
#include "PerkSystem.h"
#include "PerkModifiers.h"
#include "Serialize.h"
#include "System/File.h"
#include "System/System.h"
#include "Util/Random.h"

#include "Player/PlayerInfo.h"
#include "DamageSystem/DamageSystem.h"
#include "DamageSystem/DamageModifiers.h"
#include "Weapons/WeaponSystem.h"
#include "Weapons/IWeaponModifier.h"
#include "Weapons/Modifiers/BulletBehaviourModifiers.h"
#include "Weapons/Modifiers/DamageModifier.h"
#include "Weapons/WeaponConfiguration.h"

#include "nlohmann/json.hpp"

using namespace game;

namespace
{
    bool HasWeaponModifier(PerkType type)
    {
        switch(type)
        {
        case PerkType::IncreasedDamage:
        case PerkType::IncreasedFireRate:
        case PerkType::IncreasedBulletVelocity:
        case PerkType::IncreasedBulletSpread:
        case PerkType::IncreasedBulletRange:
        case PerkType::IncreasedMagazineSize:
        case PerkType::InfiniteAmmo:
        case PerkType::PiercingBullets:
        case PerkType::ExplosiveBullets:
        case PerkType::HomingBullets:
        case PerkType::RicochetBullets:
        case PerkType::VampiricBullets:
        case PerkType::FireBullets:
        case PerkType::IceBullets:
        case PerkType::ElectricBullets:
        case PerkType::TimeSlowBullets:
            return true;
        default:
            return false;
        }
    }

    bool HasDamageModifier(PerkType type)
    {
        switch(type)
        {
        case PerkType::ReducedDamageTaken:
        case PerkType::DodgeChance:
            return true;
        default:
            return false;
        }
    }
}

PerkSystem::PerkSystem(WeaponSystem* weapon_system, DamageSystem* damage_system)
    : m_weapon_system(weapon_system)
    , m_damage_system(damage_system)
{
    const std::vector<byte> file_data = file::FileReadAll("res/configs/perks_config.json");
    const nlohmann::json& json = nlohmann::json::parse(file_data);
    m_perk_setup = json["setup"];
    m_perk_definitions = json["perks"];

    for(uint32_t i = 0; i < (uint32_t)m_perk_definitions.size(); ++i)
    {
        if(HasWeaponModifier(m_perk_definitions[i].type))
            m_weapon_perk_indices.push_back(i);
        else if(HasDamageModifier(m_perk_definitions[i].type))
            m_damage_perk_indices.push_back(i);
    }

    if(!m_weapon_perk_indices.empty())
        m_current_enemy_perk_ids[0] = m_weapon_perk_indices.front();
    if(!m_damage_perk_indices.empty())
        m_current_enemy_perk_ids[1] = m_damage_perk_indices.front();

    m_enemy_combo_display = MakeComboDisplay(m_current_enemy_perk_ids[0], m_current_enemy_perk_ids[1]);
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
    return m_enemy_combo_display;
}

EnemyPerkModifiers PerkSystem::GetCurrentEnemyPerkModifiers() const
{
    EnemyPerkModifiers result;
    result.weapon_modifier.reset(CreateModifierForPerk(m_perk_definitions[m_current_enemy_perk_ids[0]].type));
    result.damage_modifier.reset(CreateDamageModifierForPerk(m_perk_definitions[m_current_enemy_perk_ids[1]].type));
    return result;
}

void PerkSystem::RollForNewPlayerPerk()
{
    RemoveCurrentPlayerPerk();
    m_current_player_perk_id = RerollPerkAvoidId(m_current_player_perk_id);
    ApplyPerkToPlayers(m_current_player_perk_id);
    System::Log("PerkSystem|Player perk: %s", GetCurrentPlayerPerk().name.c_str());
}

void PerkSystem::RollForNewEnemyPerk()
{
    m_current_enemy_perk_ids = RerollPerkCombo(m_current_enemy_perk_ids);
    m_enemy_combo_display = MakeComboDisplay(m_current_enemy_perk_ids[0], m_current_enemy_perk_ids[1]);
    System::Log("PerkSystem|Enemy perk: %s", m_enemy_combo_display.name.c_str());
}

uint32_t PerkSystem::RerollPerkAvoidId(uint32_t current_id) const
{
    const uint32_t n = (uint32_t)m_perk_definitions.size();
    uint32_t new_id = current_id;
    while(new_id == current_id)
        new_id = mono::Random(0u, n - 1);
    return new_id;
}

std::array<uint32_t, 2> PerkSystem::RerollPerkCombo(const std::array<uint32_t, 2>& current_ids) const
{
    const uint32_t nw = (uint32_t)m_weapon_perk_indices.size();
    uint32_t weapon_slot = current_ids[0];
    if(nw > 1)
    {
        while(weapon_slot == current_ids[0])
            weapon_slot = m_weapon_perk_indices[mono::Random(0u, nw - 1)];
    }

    const uint32_t nd = (uint32_t)m_damage_perk_indices.size();
    uint32_t damage_slot = current_ids[1];
    if(nd > 1)
    {
        while(damage_slot == current_ids[1])
            damage_slot = m_damage_perk_indices[mono::Random(0u, nd - 1)];
    }

    return { weapon_slot, damage_slot };
}

PerkDefinition PerkSystem::MakeComboDisplay(uint32_t id_a, uint32_t id_b) const
{
    const PerkDefinition& a = m_perk_definitions[id_a];
    const PerkDefinition& b = m_perk_definitions[id_b];

    PerkDefinition combo;
    combo.id = 0;
    combo.type = a.type;
    combo.name = a.adjective + " " + b.name;
    combo.description = a.description + ". " + b.description + ".";
    combo.icon_sprite_file = a.icon_sprite_file;
    return combo;
}

void PerkSystem::ApplyPerkToPlayers(uint32_t perk_id)
{
    const PerkType type = m_perk_definitions[perk_id].type;

    const PlayerArray players = GetSpawnedPlayers();
    for(const PlayerInfo* player : players)
    {
        if(!player)
            continue;

        std::unique_ptr<IWeaponModifier> weapon_modifier(CreateModifierForPerk(type));
        if(weapon_modifier)
        {
            const int slot = m_weapon_system->AddModifierForId(player->entity_id, std::move(weapon_modifier));
            m_player_modifier_slots[player->entity_id] = slot;
        }

        std::unique_ptr<IDamageModifier> damage_modifier(CreateDamageModifierForPerk(type));
        if(damage_modifier)
        {
            const int slot = m_damage_system->AddDamageModifierForId(player->entity_id, std::move(damage_modifier));
            m_player_damage_modifier_slots[player->entity_id] = slot;
        }
    }
}

void PerkSystem::RemoveCurrentPlayerPerk()
{
    for(const auto& pair : m_player_modifier_slots)
        m_weapon_system->RemoveModifierForEntity(pair.first, pair.second);
    m_player_modifier_slots.clear();

    for(const auto& pair : m_player_damage_modifier_slots)
        m_damage_system->RemoveDamageModifierForId(pair.first, pair.second);
    m_player_damage_modifier_slots.clear();
}

IDamageModifier* PerkSystem::CreateDamageModifierForPerk(PerkType type)
{
    switch(type)
    {
    case PerkType::ReducedDamageTaken:
        return new DamageReductionModifier("perk_reduced_damage_taken", 0.5f);
    case PerkType::DodgeChance:
        return new DodgeChanceModifier("perk_dodge_chance", 0.25f);
    default:
        return nullptr;
    }
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
    case PerkType::IceBullets:
        return new BulletBehaviourModifier("perk_ice_bullets", BulletCollisionFlag::SLOWS);
    default:
        return nullptr;
    }
}
