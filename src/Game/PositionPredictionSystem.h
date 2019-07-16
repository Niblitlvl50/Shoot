
#pragma once

#include "IGameSystem.h"
#include "MonoFwd.h"
#include "EventHandler/EventToken.h"
#include "Math/Vector.h"
#include <vector>

namespace mono
{
    class TransformSystem;
}

namespace game
{
    class PositionPredictionSystem : public mono::IGameSystem
    {
    public:
        
        PositionPredictionSystem(
            size_t num_records, mono::TransformSystem* transform_system, mono::EventHandler* event_handler);
        ~PositionPredictionSystem();

        uint32_t Id() const override;
        const char* Name() const override;
        uint32_t Capacity() const override;
        void Update(const mono::UpdateContext& update_context) override;

        bool HandlePredicitonMessage(const struct TransformMessage& transform_message);

        mono::TransformSystem* m_transform_system;
        mono::EventHandler* m_event_handler;

        struct PredictionData
        {
            bool is_predicting;
            math::Vector velocity;
        };

        struct KeyframeData
        {
            uint32_t entity_id;
            math::Vector position;
            float rotation;
        };

        std::vector<PredictionData> m_prediction_data;
        std::vector<KeyframeData> m_keyframe_data;
        mono::EventToken<TransformMessage> m_transform_token;
    };
}
