
#pragma once

#include "MonoFwd.h"
#include "Math/MathFwd.h"
#include <cstdint>
#include <vector>

namespace game
{
    class AnimationSystem;

    class TrainSmokeEffect
    {
    public:

        TrainSmokeEffect(
            mono::IEntityManager* entity_system,
            mono::TransformSystem* transform_system,
            mono::RenderSystem* render_system,
            mono::SpriteSystem* sprite_system,
            game::AnimationSystem* animation_system,
            uint32_t parent_entity_id);

        void UpdateEmitterSpeed(float emit_rate_per_s);
        void UpdateSmoke(const mono::UpdateContext& update_context);

    private:
        mono::IEntityManager* m_entity_system;
        mono::TransformSystem* m_transform_system;
        mono::RenderSystem* m_render_system;
        mono::SpriteSystem* m_sprite_system;
        game::AnimationSystem* m_animation_system;

        uint32_t m_parent_entity_id;
        
        struct SmokeEntity
        {
            uint32_t entity_id;
            float time_to_live_s;
            float time_to_live_counter_s;
            //mono::Color::Gradient<3> gradient;
        };
        std::vector<SmokeEntity> m_smoke_entities;
        float m_emit_rate_per_s;
        float m_emit_counter;
    };
}
