
#include "PositionPredictionSystem.h"
#include "Network/NetworkMessage.h"
#include "Network/ClientManager.h"

#include "System/Hash.h"
#include "TransformSystem/TransformSystem.h"
#include "Math/Matrix.h"
#include "System/System.h"

#include <algorithm>

using namespace game;

namespace
{
    constexpr uint16_t no_parent_16 = std::numeric_limits<uint16_t>::max();
}

PositionPredictionSystem::PositionPredictionSystem(
    size_t num_records,
    const ClientManager* client_manager,
    mono::TransformSystem* transform_system)
    : m_client_manager(client_manager)
    , m_transform_system(transform_system)
{
    m_prediction_data.resize(num_records);

    for(uint32_t index = 0; index < num_records; ++index)
        ClearPredictionsForEntity(index);
}

const char* PositionPredictionSystem::Name() const
{
    return "PositionPredictionSystem";
}

void PositionPredictionSystem::Update(const mono::UpdateContext& update_context)
{
    const int server_time = m_client_manager->GetServerTimePredicted();
    if(server_time <= 0)
        return;

    for(uint32_t index = 0; index < m_prediction_data.size(); ++index)
    {
        PredictionData& prediction_data = m_prediction_data[index];

        const RemoteTransformBuffer& prediction_buffer = prediction_data.prediction_buffer;
        if(prediction_buffer.back().timestamp == 0)
            continue;

        uint16_t new_parent_transform = no_parent_16;

        const uint32_t best_index = FindBestPredictionIndex(server_time, prediction_buffer);
        if(best_index == 0)
        {
            const RemoteTransform& to = prediction_buffer[best_index];

            prediction_data.predicted_position = to.position;
            prediction_data.predicted_rotation = to.rotation;
            new_parent_transform = to.parent_transform;
        }
        else
        {
            const RemoteTransform& from = prediction_buffer[best_index -1];
            const RemoteTransform& to = prediction_buffer[best_index];

            const float local_t = float(server_time - from.timestamp);
            const float to_from_t = float(to.timestamp - from.timestamp);
            const float t = local_t / to_from_t;

            const math::Vector& delta_position = to.position - from.position;
            const math::Vector& predicted_position = from.position + (delta_position * t);

            const float delta_rotation = to.rotation - from.rotation;
            const float predicted_rotation = from.rotation + (delta_rotation * t);

            if(from.timestamp == 0)
            {
                prediction_data.predicted_position = to.position;
                prediction_data.predicted_rotation = to.rotation;
            }
            else
            {
                prediction_data.predicted_position = predicted_position;
                prediction_data.predicted_rotation = predicted_rotation;
            }
            

            new_parent_transform = to.parent_transform;
        }

        math::Matrix& transform = m_transform_system->GetTransform(index);
        transform = math::CreateMatrixFromZRotation(prediction_data.predicted_rotation);
        math::Position(transform, prediction_data.predicted_position);

        if(new_parent_transform != no_parent_16)
            m_transform_system->ChildTransform(index, new_parent_transform);
    }
}

void PositionPredictionSystem::HandlePredicitonMessage(const TransformMessage& transform_message)
{
    PredictionData& prediction_data = m_prediction_data[transform_message.entity_id];
    RemoteTransformBuffer& prediction_buffer = prediction_data.prediction_buffer;

    if(prediction_buffer.back().timestamp < transform_message.timestamp)
    {
        // Rotate left
        std::rotate(prediction_buffer.begin(), prediction_buffer.begin() + 1, prediction_buffer.end());

        RemoteTransform& remote_transform = prediction_buffer.back();
        remote_transform.timestamp = transform_message.timestamp;
        remote_transform.position = transform_message.position;
        remote_transform.rotation = transform_message.rotation;
        remote_transform.parent_transform = transform_message.parent_transform;
    }
    else
    {
        System::Log(
            "PositionPredictionSystem|Old transform message, will skip. entity: %u have: %u new: %u",
            transform_message.entity_id,
            prediction_buffer.back().timestamp,
            transform_message.timestamp);
    }
}

void PositionPredictionSystem::ClearPredictionsForEntity(uint32_t entity_id)
{
    PredictionData& prediction_data = m_prediction_data[entity_id];
    prediction_data.predicted_position = math::ZeroVec;

    for(RemoteTransform& transform : prediction_data.prediction_buffer)
    {
        transform.timestamp = 0;
        transform.position = math::ZeroVec;
        transform.rotation = 0.0f;
        transform.parent_transform = no_parent_16;
    }
}

uint32_t PositionPredictionSystem::FindBestPredictionIndex(uint32_t timestamp, const RemoteTransformBuffer& prediction_buffer)
{
    uint32_t best_index = 0;

    for(uint32_t buffer_index = 0; buffer_index < prediction_buffer.size(); ++buffer_index)
    {
        best_index = buffer_index;
        
        const RemoteTransform& remote_transform = prediction_buffer[buffer_index];
        if(remote_transform.timestamp > timestamp)
            break;
    }

    return best_index;
}
