
#include "PositionPredictionSystem.h"
#include "Network/NetworkMessage.h"

#include "Hash.h"
#include "EventHandler/EventHandler.h"
#include "TransformSystem.h"
#include "Math/Matrix.h"

using namespace game;

PositionPredictionSystem::PositionPredictionSystem(
    size_t num_records, mono::TransformSystem* transform_system, mono::EventHandler* event_handler)
    : m_transform_system(transform_system)
    , m_event_handler(event_handler)
{
    m_prediction_data.resize(num_records);
    for(PredictionData& data : m_prediction_data)
    {
        data.time = 0;
        data.timestamp_old = 0;
        data.timestamp_new = 0;
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
    for(size_t index = 0; index < m_prediction_data.size(); ++index)
    {
        PredictionData& prediction_data = m_prediction_data[index];

        if(prediction_data.timestamp_old != 0)
        {
            prediction_data.time += update_context.delta_ms;

            const uint32_t delta_time = prediction_data.timestamp_new - prediction_data.timestamp_old;
            const float t = float(prediction_data.time) / float(delta_time);

            const math::Vector& delta_position = prediction_data.position_new - prediction_data.position_old;
            const math::Vector predicted_position = prediction_data.position_old + (delta_position * t);

            math::Matrix& transform = m_transform_system->GetTransform(index);
            transform = math::CreateMatrixFromZRotation(prediction_data.rotation);
            math::Position(transform, predicted_position);
        }
        else if(prediction_data.timestamp_new != 0)
        {
            math::Matrix& transform = m_transform_system->GetTransform(index);
            math::Position(transform, prediction_data.position_new);
        }
    }
}

bool PositionPredictionSystem::HandlePredicitonMessage(const TransformMessage& transform_message)
{
    PredictionData& prediction_data = m_prediction_data[transform_message.entity_id];

    if(transform_message.timestamp > prediction_data.timestamp_new)
    {
        prediction_data.time = 0;

        prediction_data.timestamp_old = prediction_data.timestamp_new;
        prediction_data.timestamp_new = transform_message.timestamp;

        prediction_data.position_old = prediction_data.position_new;
        prediction_data.position_new = transform_message.position;

        prediction_data.rotation = transform_message.rotation;
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
    std::vector<math::Vector> end_points;
    std::vector<math::Vector> line_points;

    for(const auto& prediction : m_prediction_system->m_prediction_data)
    {
        end_points.push_back(prediction.position_new);

        line_points.push_back(prediction.position_old);
        line_points.push_back(prediction.position_new);
    }
    
    renderer.DrawLines(line_points, mono::Color::BLUE, 2.0f);
    renderer.DrawPoints(end_points, mono::Color::GREEN, 4.0f);
}

math::Quad PredictionSystemDebugDrawer::BoundingBox() const
{
    return math::InfQuad;
}
