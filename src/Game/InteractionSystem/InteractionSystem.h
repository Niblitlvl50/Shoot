
#pragma once

#include "MonoFwd.h"
#include "IGameSystem.h"
#include <vector>

namespace game
{
    struct InteractionComponent
    {
        uint32_t on_interaction_hash;
        uint32_t off_interaction_hash;
        bool triggered;
    };

    struct InteractionAndTrigger
    {
        uint32_t interaction_id;
        uint32_t trigger_id;
    };

    struct FrameInteractionData
    {
        std::vector<InteractionAndTrigger> active;
        std::vector<InteractionAndTrigger> deactivated;
    };

    class InteractionSystem : public mono::IGameSystem
    {
    public:

        InteractionSystem(uint32_t n, mono::TransformSystem* transform_system, class TriggerSystem* trigger_system);

        InteractionComponent* AllocateComponent(uint32_t entity_id);
        void ReleaseComponent(uint32_t entity_id);
        void AddComponent(uint32_t entity_id, uint32_t interaction_hash);
        void AddComponent(uint32_t entity_id, uint32_t on_interaction_hash, uint32_t off_interaction_hash);

        uint32_t Id() const override;
        const char* Name() const override;
        void Update(const mono::UpdateContext& update_context) override;

        void TryTriggerInteraction(uint32_t entity_id);

        const FrameInteractionData& GetFrameInteractionData() const;

        template <typename T>
        inline void ForEach(T&& callable)
        {
            for(uint32_t index = 0; index < m_active.size(); ++index)
            {
                const bool is_active = m_active[index];
                if(is_active)
                    callable(index, m_components[index]);
            }
        }

    private:

        mono::TransformSystem* m_transform_system;
        game::TriggerSystem* m_trigger_system;

        std::vector<InteractionComponent> m_components;
        std::vector<bool> m_active;

        std::vector<InteractionAndTrigger> m_previous_active_interactions;
        FrameInteractionData m_interaction_data;

        std::vector<uint32_t> m_player_triggers;
    };
}
