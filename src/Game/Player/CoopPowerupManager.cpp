
#include "CoopPowerupManager.h"
#include "DamageSystem.h"
#include "PlayerInfo.h"

#include "Math/MathFunctions.h"

namespace tweak_values
{
    constexpr float decay_per_second = 10.0f;
    constexpr float power_up_threshold = 1000.0;
}

using namespace game;

CoopPowerupManager::CoopPowerupManager(game::DamageSystem* damage_system)
    : m_damage_system(damage_system)
    , m_powerup_value_raw(0.0f)
    , m_powerup_value_01(0.0f)
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
    const float frame_decay = update_context.delta_s * tweak_values::decay_per_second;
    m_powerup_value_raw = m_powerup_value_raw - frame_decay;
    m_powerup_value_01 = math::Scale01Clamped(m_powerup_value_raw, 0.0f, tweak_values::power_up_threshold);

    game::g_coop_powerup_value = m_powerup_value_01;
}
