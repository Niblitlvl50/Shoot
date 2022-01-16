
#pragma once

#include "IGameSystem.h"
#include "MonoFwd.h"
#include "Math/Vector.h"
#include "Math/EasingFunctions.h"
#include "Util/ObjectPool.h"
#include "AnimationModes.h"

#include <vector>

namespace game
{
    struct SpriteAnimationComponent
    {
        uint32_t target_id;
        uint32_t trigger_hash;
        uint32_t callback_id;

        uint32_t animation_index;
    };

    enum class TransformAnimType
    {
        TRANSLATION,
        ROTATION
    };

    struct TransformAnimationComponent
    {
        uint32_t target_id;
        uint32_t trigger_hash;
        uint32_t callback_id;

        float duration;
        float duration_counter;

        math::EaseFunction ease_function;
        AnimationMode animation_flags;
        TransformAnimType transform_type;

        bool is_initialized;

        float start_x;
        float start_y;

        float delta_x;
        float delta_y;
    };

    struct AnimationContainer
    {
        int ref_counter = 0;
        std::vector<SpriteAnimationComponent*> sprite_components;
        std::vector<TransformAnimationComponent*> transform_components;
    };

    class AnimationSystem : public mono::IGameSystem
    {
    public:

        AnimationSystem(
            uint32_t n, class TriggerSystem* trigger_system, mono::TransformSystem* transform_system, mono::SpriteSystem* sprite_system);

        AnimationContainer* AllocateAnimationContainer(uint32_t entity_id);
        void ReleaseAnimationContainer(uint32_t entity_id);
        bool IsAnimationContainerAllocated(uint32_t entity_id);

        SpriteAnimationComponent* AddSpriteAnimation(
            uint32_t container_id,
            uint32_t trigger_hash,
            uint32_t animation_index);

        TransformAnimationComponent* AddTranslationComponent(
            uint32_t container_id,
            uint32_t trigger_hash,
            float duration,
            math::EaseFunction func,
            AnimationMode mode,
            const math::Vector& translation_delta);

        TransformAnimationComponent* AddRotationComponent(
            uint32_t container_id,
            uint32_t trigger_hash,
            float duration,
            math::EaseFunction func,
            AnimationMode mode,
            float rotation_delta);

        uint32_t Id() const override;
        const char* Name() const override;
        void Update(const mono::UpdateContext& update_context) override;

        void AddTransformAnimatonToUpdate(TransformAnimationComponent* transform_animation);

        class TriggerSystem* m_trigger_system;
        mono::TransformSystem* m_transform_system;
        mono::SpriteSystem* m_sprite_system;

        mono::ObjectPool<SpriteAnimationComponent> m_sprite_anim_pool;
        mono::ObjectPool<TransformAnimationComponent> m_transform_anim_pool;

        std::vector<AnimationContainer> m_animation_containers;
        std::vector<bool> m_active_animation_containers;

        std::vector<SpriteAnimationComponent*> m_sprite_anims_to_process;
        std::vector<TransformAnimationComponent*> m_transform_anims_to_process;
    };
}
