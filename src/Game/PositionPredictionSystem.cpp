
#include "PositionPredictionSystem.h"
#include "Network/NetworkMessage.h"
#include "Network/ClientManager.h"

#include "Hash.h"
#include "EventHandler/EventHandler.h"
#include "TransformSystem/TransformSystem.h"
#include "Math/Matrix.h"
#include "System/System.h"
#include <algorithm>

using namespace game;

PositionPredictionSystem::PositionPredictionSystem(
    size_t num_records,
    const ClientManager* client_manager,
    mono::TransformSystem* transform_system,
    mono::EventHandler* event_handler)
    : m_client_manager(client_manager)
    , m_transform_system(transform_system)
    , m_event_handler(event_handler)
{
    m_prediction_data.resize(num_records);
    for(PredictionData& prediction_data : m_prediction_data)
    {
        for(RemoteTransform& transform : prediction_data.prediction_buffer)
        {
            transform.timestamp = 0;
            transform.position = math::ZeroVec;
            transform.rotation = 0.0f;
        }
    }

    using namespace std::placeholders;
    std::function<bool (const TransformMessage&)> transform_func = std::bind(&PositionPredictionSystem::HandlePredicitonMessage, this, _1);
    m_transform_token = m_event_handler->AddListener(transform_func);
}

PositionPredictionSystem::~PositionPredictionSystem()
{
    m_event_handler->RemoveListener(m_transform_token);
}

uint32_t PositionPredictionSystem::Id() const
{
    return mono::Hash(Name());
}

const char* PositionPredictionSystem::Name() const
{
    return "PositionPredictionSystem";
}

uint32_t PositionPredictionSystem::Capacity() const
{
    return m_prediction_data.size();
}

void PositionPredictionSystem::Update(const mono::UpdateContext& update_context)
{
    const int server_time = m_client_manager->GetServerTimePredicted();
    if(server_time <= 0)
        return;

    const auto find_remote_transform = [server_time](const RemoteTransform& remote_transform) {
        return server_time < int(remote_transform.timestamp);
    };

    for(size_t index = 0; index < m_prediction_data.size(); ++index)
    {
        PredictionData& prediction_data  = m_prediction_data[index];

        const auto& prediction_buffer = prediction_data.prediction_buffer;
        if(prediction_buffer.back().timestamp == 0)
            continue;

        const auto it = std::find_if(prediction_buffer.begin(), prediction_buffer.end(), find_remote_transform);
        if(it == prediction_buffer.end())
            continue;

        const RemoteTransform& from = *(it -1);
        const RemoteTransform& to = *it;

        const float local_t = float(server_time - from.timestamp);
        const float to_from_t = float(to.timestamp - from.timestamp);
        const float t = local_t / to_from_t; 

        const math::Vector& delta_position = to.position - from.position;
        const math::Vector& predicted_position = from.position + (delta_position * t);

        const float delta_rotation = to.rotation - from.rotation;
        const float predicted_rotation = from.rotation + (delta_rotation * t);

        prediction_data.predicted_position = predicted_position;

        math::Matrix& transform = m_transform_system->GetTransform(index);
        transform = math::CreateMatrixFromZRotation(predicted_rotation);
        math::Position(transform, predicted_position);

        m_transform_system->ChildTransform(index, from.parent_transform);
    }
}

bool PositionPredictionSystem::HandlePredicitonMessage(const TransformMessage& transform_message)
{
    PredictionData& prediction_data = m_prediction_data[transform_message.entity_id];
    auto& prediction_buffer = prediction_data.prediction_buffer;

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
        System::Log("network|Old transform message, will skip\n");
    }

    return false;
}



#include "Math/Quad.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"

PredictionSystemDebugDrawer::PredictionSystemDebugDrawer(const PositionPredictionSystem* prediction_system)
    : m_prediction_system(prediction_system)
{ }

void PredictionSystemDebugDrawer::doDraw(mono::IRenderer& renderer) const
{
    std::vector<math::Vector> line_points;
    std::vector<math::Vector> first_points;
    std::vector<math::Vector> predicted_positions;

    for(const auto& prediction_data : m_prediction_system->m_prediction_data)
    {
        for(const auto& remote_transform : prediction_data.prediction_buffer)
            line_points.push_back(remote_transform.position);

        first_points.push_back(prediction_data.prediction_buffer.back().position);
        predicted_positions.push_back(prediction_data.predicted_position);
    }
    
    renderer.DrawPoints(line_points, mono::Color::GREEN, 4.0f);
    renderer.DrawPoints(first_points, mono::Color::RED, 6.0f);
    renderer.DrawPoints(predicted_positions, mono::Color::CYAN, 6.0f);
}

math::Quad PredictionSystemDebugDrawer::BoundingBox() const
{
    return math::InfQuad;
}
