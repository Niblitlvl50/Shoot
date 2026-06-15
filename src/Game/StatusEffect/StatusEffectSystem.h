
#pragma once

#include "IGameSystem.h"

#include <cstdint>
#include <unordered_map>

namespace mono
{
    class PhysicsSystem;
}

namespace game
{
    class StatusEffectSystem : public mono::IGameSystem
    {
    public:

        StatusEffectSystem(mono::PhysicsSystem* physics_system);

        void ApplySlowEffect(uint32_t entity_id, float multiplier, float duration_s);
        void ClearEffects(uint32_t entity_id);

        const char* Name() const override;
        void Update(const mono::UpdateContext& update_context) override;

    private:

        struct SlowEffect
        {
            float multiplier;
            float remaining_s;
        };

        mono::PhysicsSystem* m_physics_system;
        std::unordered_map<uint32_t, SlowEffect> m_slow_effects;
    };
}
