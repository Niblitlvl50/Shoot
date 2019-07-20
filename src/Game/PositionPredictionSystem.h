
#pragma once

#include "IGameSystem.h"
#include "MonoFwd.h"
#include "EventHandler/EventToken.h"
#include "Math/Vector.h"
#include <vector>

#include "Rendering/IDrawable.h"

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
        mono::EventToken<TransformMessage> m_transform_token;

        struct PredictionData
        {
            uint32_t time;
            uint32_t timestamp_old;
            uint32_t timestamp_new;
            math::Vector position_old;
            math::Vector position_new;
            float rotation;
        };

        std::vector<PredictionData> m_prediction_data;
    };


    class PredictionSystemDebugDrawer : public mono::IDrawable
    {
    public:

        PredictionSystemDebugDrawer(const PositionPredictionSystem* prediction_system);
        void doDraw(mono::IRenderer& renderer) const;
        math::Quad BoundingBox() const;

        const PositionPredictionSystem* m_prediction_system;
    };
}
