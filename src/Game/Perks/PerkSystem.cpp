
#include "PerkSystem.h"
#include "Serialize.h"
#include "System/File.h"
#include "Util/Random.h"

#include "nlohmann/json.hpp"


using namespace game;

PerkSystem::PerkSystem()
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

    /*
    if(m_current_reroll_time > m_perk_setup.reroll_duration)
    {
        m_current_reroll_time = 0.0f;
        RollForNewPlayerPerk();
        RollForNewEnemyPerk();
    }
    */
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
    m_current_player_perk_id = RerollPerkAvoidId(m_current_player_perk_id);
}

void PerkSystem::RollForNewEnemyPerk()
{
    m_current_enemy_perk_id = RerollPerkAvoidId(m_current_enemy_perk_id);
}

uint32_t PerkSystem::RerollPerkAvoidId(uint32_t perk_id_to_avoid)
{
    uint32_t new_perk_id = perk_id_to_avoid;
    while(new_perk_id == perk_id_to_avoid)
    {
        new_perk_id = mono::Random(0, m_perk_definitions.size() - 1);
    }

    return new_perk_id;
}
