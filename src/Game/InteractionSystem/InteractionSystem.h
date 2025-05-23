
#pragma once

#include "MonoFwd.h"
#include "IGameSystem.h"
#include "InteractionType.h"
#include "Input/InputSystemTypes.h"
#include "Util/ActiveVector.h"
#include "System/Audio.h"

#include <vector>
#include <functional>
#include <string>

namespace game
{
    struct InteractionComponent
    {
        uint32_t on_interaction_hash;
        uint32_t off_interaction_hash;
        InteractionType type;
        bool draw_name;
        audio::ISoundPtr sound;
    };

    struct InteractionAndTrigger
    {
        uint32_t interaction_id;
        uint32_t trigger_id;
        InteractionType interaction_type;
        mono::InputContextType input_type;
        bool draw_name;
    };

    struct FrameInteractionData
    {
        std::vector<InteractionAndTrigger> active;
        std::vector<InteractionAndTrigger> deactivated;
    };

    using InteractionCallback = std::function<void (uint32_t entity_id, InteractionType interaction_type)>;

    class InteractionSystem : public mono::IGameSystem
    {
    public:

        InteractionSystem(uint32_t n, mono::TransformSystem* transform_system, mono::TriggerSystem* trigger_system);

        InteractionComponent* AllocateComponent(uint32_t entity_id);
        void ReleaseComponent(uint32_t entity_id);
        void AddComponent(
            uint32_t entity_id,
            uint32_t interaction_hash,
            InteractionType interaction_type,
            bool draw_name,
            const std::string& interaction_sound);
        void AddComponent(
            uint32_t entity_id,
            uint32_t on_interaction_hash,
            uint32_t off_interaction_hash,
            InteractionType interaction_type,
            bool draw_name,
            const std::string& interaction_sound);

        const char* Name() const override;
        void Update(const mono::UpdateContext& update_context) override;

        void TryTriggerInteraction(uint32_t entity_id, const InteractionCallback& callback);
        bool CanPlayerTriggerInteraction(uint32_t player_entity_id);
        void SetInteractionEnabled(uint32_t entity_id, bool enabled);
        void SetInteractionCallback(uint32_t entity_id, const InteractionCallback& callback);

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
            InteractionCallback callback;
        };

        mono::TransformSystem* m_transform_system;
        mono::TriggerSystem* m_trigger_system;

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
