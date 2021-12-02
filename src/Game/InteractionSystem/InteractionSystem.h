
#pragma once

#include "MonoFwd.h"
#include "IGameSystem.h"
#include "InteractionType.h"
#include "Util/ActiveVector.h"
#include <vector>
#include <functional>

namespace game
{
    struct InteractionComponent
    {
        uint32_t on_interaction_hash;
        uint32_t off_interaction_hash;
        shared::InteractionType type;
        bool draw_name;
    };

    struct InteractionAndTrigger
    {
        uint32_t interaction_id;
        uint32_t trigger_id;
        shared::InteractionType interaction_type;
        bool draw_name;
    };

    struct FrameInteractionData
    {
        std::vector<InteractionAndTrigger> active;
        std::vector<InteractionAndTrigger> deactivated;
    };

    using InteractionCallback = std::function<void (uint32_t entity_id, shared::InteractionType interaction_type)>;

    class InteractionSystem : public mono::IGameSystem
    {
    public:

        InteractionSystem(uint32_t n, mono::TransformSystem* transform_system, class TriggerSystem* trigger_system);

        InteractionComponent* AllocateComponent(uint32_t entity_id);
        void ReleaseComponent(uint32_t entity_id);
        void AddComponent(uint32_t entity_id, uint32_t interaction_hash, shared::InteractionType interaction_type, bool draw_name);
        void AddComponent(uint32_t entity_id, uint32_t on_interaction_hash, uint32_t off_interaction_hash, shared::InteractionType interaction_type, bool draw_name);

        uint32_t Id() const override;
        const char* Name() const override;
        void Update(const mono::UpdateContext& update_context) override;

        void TryTriggerInteraction(uint32_t entity_id, const InteractionCallback& callback);
        bool CanPlayerTriggerInteraction(uint32_t player_entity_id);
        void SetInteractionEnabled(uint32_t entity_id, bool enabled);

        const FrameInteractionData& GetFrameInteractionData() const;

        template <typename T>
        inline void ForEach(T&& callable)
        {
            m_components.ForEach(callable);
        }

    private:

        struct InteractionComponentDetails
        {
            bool triggered;
            bool enabled;
        };

        mono::TransformSystem* m_transform_system;
        game::TriggerSystem* m_trigger_system;

        mono::ActiveVector<InteractionComponent> m_components;
        std::vector<InteractionComponentDetails> m_component_details;

        std::vector<InteractionAndTrigger> m_previous_active_interactions;
        FrameInteractionData m_interaction_data;

        struct PlayerTriggerData
        {
            uint32_t player_entity_id;
            InteractionCallback callback;
        };
        std::vector<PlayerTriggerData> m_player_triggers;
    };
}
