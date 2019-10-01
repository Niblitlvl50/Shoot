
#include "PositionPredictionSystem.h"
#include "Network/NetworkMessage.h"

#include "Hash.h"
#include "EventHandler/EventHandler.h"
#include "TransformSystem.h"
#include "Math/Matrix.h"

#include <algorithm>

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
        data.timestamp_start = 0;
        data.timestamp_end = 0;
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
    //for(size_t index = 0; index < 1; ++index)
    {
        PredictionData& prediction_data = m_prediction_data[index];

        math::Vector predicted_position = prediction_data.position_end;
        float predicted_rotation = prediction_data.rotation_end;

        if(prediction_data.timestamp_start != 0)
        {
            prediction_data.time += update_context.delta_ms;

            const uint32_t delta_time = prediction_data.timestamp_end - prediction_data.timestamp_start;
            const float t = float(prediction_data.time) / float(delta_time);
            prediction_data.t = t;

            /*
            const char* prefix = t > 1.0f ? " ----> " : "";
            std::printf(
                "%s T: %f | prediction time: %u, start: %u, end: %u, delta: %u\n",
                prefix,
                t,
                prediction_data.time,
                prediction_data.timestamp_start,
                prediction_data.timestamp_end,
                delta_time);
            */

            // This perhaps
            // entity.x = x0 + (x1 - x0) * (render_timestamp - t0) / (t1 - t0);

            // const float t =
            // prediction_data.position_start + (delta_position * (update_context.total_time - prediction_data.timestamp_start) / delta_time);

            const math::Vector& delta_position = prediction_data.position_end - prediction_data.position_start;
            predicted_position = prediction_data.position_start + (delta_position * t);

            const float delta_rotation = prediction_data.rotation_end - prediction_data.rotation_start;
            predicted_rotation = prediction_data.rotation_start + (delta_rotation * t);
        }

        math::Matrix& transform = m_transform_system->GetTransform(index);
        transform = math::CreateMatrixFromZRotation(predicted_rotation);
        math::Position(transform, predicted_position);
    }
}

bool PositionPredictionSystem::HandlePredicitonMessage(const TransformMessage& transform_message)
{
    PredictionData& prediction_data = m_prediction_data[transform_message.entity_id];

    if(transform_message.timestamp > prediction_data.timestamp_end)
    {
        const math::Matrix& transform = m_transform_system->GetTransform(transform_message.entity_id);
        const math::Vector& actual_position = math::GetPosition(transform);

        prediction_data.time = 0;

        prediction_data.timestamp_start = prediction_data.timestamp_end;
        prediction_data.timestamp_end = transform_message.timestamp;

        prediction_data.position_start = actual_position;
        prediction_data.position_end = transform_message.position;

        prediction_data.rotation_start = prediction_data.rotation_end;
        prediction_data.rotation_end = transform_message.rotation;
    }
    else
    {
        std::printf("PositionReplicationSystem|Old transform message, will dump.\n");
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
    std::vector<math::Vector> over_predicted_points;
    std::vector<math::Vector> line_points;

    for(const auto& prediction_data : m_prediction_system->m_prediction_data)
    {
        if(prediction_data.t > 1.0f)
            over_predicted_points.push_back(prediction_data.position_end);
        else
            end_points.push_back(prediction_data.position_end);

        line_points.push_back(prediction_data.position_start);
        line_points.push_back(prediction_data.position_end);
    }
    
    renderer.DrawLines(line_points, mono::Color::BLUE, 2.0f);
    renderer.DrawPoints(end_points, mono::Color::GREEN, 4.0f);
    renderer.DrawPoints(over_predicted_points, mono::Color::RED, 6.0f);
}

math::Quad PredictionSystemDebugDrawer::BoundingBox() const
{
    return math::InfQuad;
}
