
#pragma once

#include "IGameSystem.h"
#include "MonoFwd.h"
#include "Util/ObjectPool.h"
#include "Math/Vector.h"
#include "Math/EasingFunctions.h"

#include <vector>

namespace game
{
   struct TranslationComponent
   {
       float start_x;
       float start_y;
       float delta_x;
       float delta_y;
       bool ping_pong;
   };

   struct RotationComponent
   {
       float start_rotation;
       float delta_rotation;
   };

    struct ModificationType
    {
        uint32_t target_id;
        uint32_t trigger_hash;
        uint32_t callback_id;

        float duration;
        float duration_counter;

        math::EaseFunction ease_function;

        bool is_initialized;

        enum class Type : short
        {
            ANIMATION,
            ROTATION,
            TRANSLATION,
        };

        Type type = Type::ANIMATION;

        union
        {
            uint32_t animation_index;
            RotationComponent rotation;
            TranslationComponent translation;
        };
    };

    struct ModificationContainer
    {
        std::vector<const ModificationType*> modifications;
    };

    class ModificationSystem : public mono::IGameSystem
    {
    public:

        ModificationSystem(
            uint32_t n, class TriggerSystem* trigger_system, mono::TransformSystem* transform_system, mono::SpriteSystem* sprite_system);

        void AllocateContainer(uint32_t id);
        void ReleaseContainer(uint32_t id);
        bool IsContainerAllocated(uint32_t id);

        void AddAnimationComponent(uint32_t container_id, uint32_t trigger_hash, uint32_t animation_index);
        void AddTranslationComponent(uint32_t container_id, uint32_t trigger_hash, float duration, math::EaseFunction func, const math::Vector& translation_delta);
        void AddRotationComponent(uint32_t container_id, uint32_t trigger_hash, float duration, math::EaseFunction func, float rotation_delta);

        uint32_t Id() const override;
        const char* Name() const override;
        uint32_t Capacity() const override;
        void Update(const mono::UpdateContext& update_context) override;

        class TriggerSystem* m_trigger_system;
        mono::TransformSystem* m_transform_system;
        mono::SpriteSystem* m_sprite_system;

        std::vector<ModificationContainer> m_containers;
        std::vector<bool> m_active_containers;
        mono::ObjectPool<ModificationType> m_modification_components;

        std::vector<ModificationType*> m_active_modifications;
    };
}
