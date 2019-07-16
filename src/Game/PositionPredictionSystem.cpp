
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
        data.is_predicting = false;

    m_keyframe_data.reserve(128);

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
        const PredictionData& prediciton_data = m_prediction_data[index];
        if(prediciton_data.is_predicting)
        {
            math::Matrix& transform = m_transform_system->GetTransform(index);

            const math::Vector& new_position =
                math::GetPosition(transform) + (prediciton_data.velocity * float(update_context.delta_ms) / 1000.0f);

            math::Position(transform, new_position);
        }
    }

    for(const KeyframeData& keyframe : m_keyframe_data)
    {
        math::Matrix& transform = m_transform_system->GetTransform(keyframe.entity_id);
        transform = math::CreateMatrixFromZRotation(keyframe.rotation);
        math::Position(transform, keyframe.position);
    }

    m_keyframe_data.clear();
}

bool PositionPredictionSystem::HandlePredicitonMessage(const TransformMessage& transform_message)
{
    m_prediction_data[transform_message.entity_id].is_predicting = true;
    m_prediction_data[transform_message.entity_id].velocity = transform_message.velocity;

    KeyframeData keyframe;
    keyframe.entity_id = transform_message.entity_id;
    keyframe.position = transform_message.position;
    keyframe.rotation = transform_message.rotation;
    m_keyframe_data.push_back(keyframe);

    return false;
}
