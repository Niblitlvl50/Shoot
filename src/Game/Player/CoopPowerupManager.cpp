
#include "CoopPowerupManager.h"
#include "DamageSystem/DamageSystem.h"
#include "PlayerInfo.h"

#include "Math/MathFunctions.h"
#include "System/System.h"

#include <algorithm>

namespace tweak_values
{
    constexpr float decay_per_second = 5.0f;
    constexpr float power_up_threshold = 500.0;
    constexpr float time_activated_s = 5.0f;
}

using namespace game;

CoopPowerupManager::CoopPowerupManager(game::DamageSystem* damage_system)
    : m_damage_system(damage_system)
    , m_powerup_value_raw(0.0f)
    , m_activated_timer_s(0.0f)
{
    const DamageCallback on_entity_destroyed = [this](uint32_t id, int damage, uint32_t who_did_damage, DamageType type) {
        const game::DamageRecord* damage_record = m_damage_system->GetDamageRecord(id);
        const float multiplier = damage_record->is_boss ? 1.5f : 1.0f;
        m_powerup_value_raw += (damage_record->full_health * multiplier);
    };

    m_damage_callback_id = m_damage_system->SetGlobalDamageCallback(DamageType::DESTROYED, on_entity_destroyed);
}

CoopPowerupManager::~CoopPowerupManager()
{
    m_damage_system->RemoveGlobalDamageCallback(m_damage_callback_id);
}

void CoopPowerupManager::Update(const mono::UpdateContext& update_context)
{
    switch(g_coop_powerup.state)
    {
    case CoopPowerUpState::DISABLED:
    {
        const auto not_nullptr = [](const game::PlayerInfo* player_info) {
            return player_info != nullptr;
        };

        const PlayerArray spawned_players = game::GetSpawnedPlayers();
        const int num_players = std::count_if(spawned_players.begin(), spawned_players.end(), not_nullptr);
        if(num_players > 1)
            g_coop_powerup.state = CoopPowerUpState::ENABLED;

        break;
    }
    case CoopPowerUpState::ENABLED:
    {
        const float frame_decay = update_context.delta_s * tweak_values::decay_per_second;
        m_powerup_value_raw = std::clamp(m_powerup_value_raw - frame_decay, 0.0f, tweak_values::power_up_threshold);
        if(m_powerup_value_raw >= tweak_values::power_up_threshold)
        {
            g_coop_powerup.state = CoopPowerUpState::TRIGGERED;
            // Trigger powerup here
        }

        break;
    }
    case CoopPowerUpState::TRIGGERED:
    {
        m_activated_timer_s += update_context.delta_s;

        if(m_activated_timer_s >= tweak_values::time_activated_s)
        {
            m_activated_timer_s = 0.0f;
            m_powerup_value_raw = 0.0f;
            g_coop_powerup.state = CoopPowerUpState::ENABLED;
        }

        break;
    }
    }

    const float powerup_value_01 = math::Scale01Clamped(m_powerup_value_raw, 0.0f, tweak_values::power_up_threshold);
    g_coop_powerup.powerup_value = powerup_value_01;
}
