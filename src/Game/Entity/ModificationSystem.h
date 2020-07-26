
#pragma once

#include "IGameSystem.h"
#include "MonoFwd.h"
#include <vector>

namespace game
{
    struct AnimationComponent
    {
        uint32_t trigger_hash;
        uint32_t animation_index;
    };

    class ModificationSystem : public mono::IGameSystem
    {
    public:

        ModificationSystem(uint32_t n, class TriggerSystem* trigger_system, mono::SpriteSystem* sprite_system);

        AnimationComponent* AllocateAnimationComponent(uint32_t id);
        void ReleaseAnimationComponent(uint32_t id);
        void UpdateAnimationComponent(uint32_t id, const AnimationComponent& animation_data);

        uint32_t Id() const override;
        const char* Name() const override;
        uint32_t Capacity() const override;
        void Update(const mono::UpdateContext& update_context) override;

        class TriggerSystem* m_trigger_system;
        mono::SpriteSystem* m_sprite_system;

        std::vector<AnimationComponent> m_animations;
        std::vector<bool> m_active_animations;

        std::vector<uint32_t> m_callback_ids;

        struct AnimationUpdate
        {
            uint32_t target_id;
            uint32_t animation_index;
        };
        std::vector<AnimationUpdate> m_animations_to_update;
    };
}
