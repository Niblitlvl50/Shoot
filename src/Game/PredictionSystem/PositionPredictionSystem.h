
#pragma once

#include "IGameSystem.h"
#include "MonoFwd.h"
#include "Math/Vector.h"

#include <vector>
#include <array>


namespace game
{
    class ClientManager;

    class PositionPredictionSystem : public mono::IGameSystem
    {
    public:
        
        PositionPredictionSystem(
            size_t num_records,
            const ClientManager* client_manager,
            mono::TransformSystem* transform_system);

        uint32_t Id() const override;
        const char* Name() const override;
        void Update(const mono::UpdateContext& update_context) override;

        void HandlePredicitonMessage(const struct TransformMessage& transform_message);
        void ClearPredictionsForEntity(uint32_t entity_id);

        const ClientManager* m_client_manager;
        mono::TransformSystem* m_transform_system;

        struct RemoteTransform
        {
            uint32_t timestamp;
            math::Vector position;
            float rotation;
            uint16_t parent_transform;
        };

        struct PredictionData
        {
            math::Vector predicted_position;
            std::array<RemoteTransform, 8> prediction_buffer;
        };

        std::vector<PredictionData> m_prediction_data;
    };
}
