
#include "PositionPredictionSystemDebug.h"
#include "PositionPredictionSystem.h"

#include "Math/Quad.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"
#include "Debug/GameDebugVariables.h"

using namespace game;

PredictionSystemDebugDrawer::PredictionSystemDebugDrawer(const PositionPredictionSystem* prediction_system)
    : m_prediction_system(prediction_system)
{ }

void PredictionSystemDebugDrawer::Draw(mono::IRenderer& renderer) const
{
    if(!game::g_draw_position_prediction)
        return;

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
