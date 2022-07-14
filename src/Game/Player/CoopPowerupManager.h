
#pragma once

#include "IUpdatable.h"
#include <cstdint>

namespace game
{
    class DamageSystem;

    class CoopPowerupManager : public mono::IUpdatable
    {
    public:

        CoopPowerupManager(game::DamageSystem* damage_system);
        ~CoopPowerupManager();

        void Update(const mono::UpdateContext& update_context) override;

        game::DamageSystem* m_damage_system;
        uint32_t m_damage_callback_id;

        float m_powerup_value;
    };
}
