
#pragma once

#include "IGameSystem.h"
#include "EntitySystem/Entity.h"

#include <cstdint>
#include <string>
#include <unordered_map>

namespace mono
{
    class PhysicsSystem;
}

namespace game
{
    class EntityAnnotationSystem;

    class StatusEffectSystem : public mono::IGameSystem
    {
    public:

        StatusEffectSystem(mono::PhysicsSystem* physics_system, game::EntityAnnotationSystem* annotation_system);

        void ApplySlowEffect(uint32_t entity_id, float multiplier, float duration_s);
        void ClearEffects(uint32_t entity_id);

        const char* Name() const override;
        void Update(const mono::UpdateContext& update_context) override;

    private:

        struct SlowEffect
        {
            float multiplier;
            float remaining_s;
            uint32_t annotation_id;
        };

        mono::PhysicsSystem* m_physics_system;
        game::EntityAnnotationSystem* m_annotation_system;
        std::unordered_map<uint32_t, SlowEffect> m_slow_effects;
        std::string m_slow_annotation_entity;
    };
}
