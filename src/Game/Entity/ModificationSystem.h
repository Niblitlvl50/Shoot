
#pragma once

#include "IGameSystem.h"
#include "MonoFwd.h"
#include "Util/ObjectPool.h"
#include "Math/Vector.h"
#include "Math/EasingFunctions.h"

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

    struct TranslateAnimationComponent
    {
        uint32_t target_id;
        uint32_t trigger_hash;
        uint32_t callback_id;

        float duration;
        float duration_counter;

        math::EaseFunction ease_function;
        shared::AnimationMode animation_flags;

        bool is_initialized;

        float start_x;
        float start_y;
        float delta_x;
        float delta_y;
    };

    struct RotationAnimationComponent
    {
        uint32_t target_id;
        uint32_t trigger_hash;
        uint32_t callback_id;

        float duration;
        float duration_counter;

        math::EaseFunction ease_function;
        shared::AnimationMode animation_flags;

        bool is_initialized;

        float start_rotation;
        float delta_rotation;
    };

    class ModificationSystem : public mono::IGameSystem
    {
    public:

        ModificationSystem(
            uint32_t n, class TriggerSystem* trigger_system, mono::TransformSystem* transform_system, mono::SpriteSystem* sprite_system);

        SpriteAnimationComponent* AllocateSpriteAnimation(uint32_t entity_id);
        void ReleaseSpriteAnimation(uint32_t entity_id);
        void AddAnimationComponent(uint32_t entity_id, uint32_t trigger_hash, uint32_t animation_index);

        TranslateAnimationComponent* AllocateTranslationAnimation(uint32_t entity_id);
        void ReleaseTranslationAnimation(uint32_t entity_id);
        void AddTranslationComponent(
            uint32_t container_id, uint32_t trigger_hash, float duration, math::EaseFunction func, shared::AnimationMode mode, const math::Vector& translation_delta);

        RotationAnimationComponent* AllocateRotationAnimation(uint32_t entity_id);
        void ReleaseRotationAnimation(uint32_t entity_id);
        void AddRotationComponent(
            uint32_t container_id, uint32_t trigger_hash, float duration, math::EaseFunction func, shared::AnimationMode mode, float rotation_delta);

        uint32_t Id() const override;
        const char* Name() const override;
        uint32_t Capacity() const override;
        void Update(const mono::UpdateContext& update_context) override;

        class TriggerSystem* m_trigger_system;
        mono::TransformSystem* m_transform_system;
        mono::SpriteSystem* m_sprite_system;

        std::vector<SpriteAnimationComponent> m_sprite_components;
        std::vector<bool> m_active_sprite_components;

        std::vector<TranslateAnimationComponent> m_translation_components;
        std::vector<bool> m_active_translation_components;

        std::vector<RotationAnimationComponent> m_rotation_components;
        std::vector<bool> m_active_rotation_components;

        std::vector<SpriteAnimationComponent*> m_sprite_anims_to_process;
        std::vector<TranslateAnimationComponent*> m_translation_anims_to_process;
        std::vector<RotationAnimationComponent*> m_rotation_anims_to_process;
    };
}
