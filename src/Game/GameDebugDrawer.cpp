
#include "GameDebugDrawer.h"
#include "Math/Quad.h"
#include "Math/Matrix.h"
#include "Rendering/IRenderer.h"
#include "Util/Algorithm.h"

#include "Factories.h"

using namespace game;

namespace
{
    template <typename T>
    bool remove_if_5(const T& item)
    {
        return item.timestamp > 5000;
    }
}

GameDebugDrawer::GameDebugDrawer()
{
    game::g_debug_drawer = this;
}

GameDebugDrawer::~GameDebugDrawer()
{
    game::g_debug_drawer = nullptr;
}

void GameDebugDrawer::Draw(mono::IRenderer& renderer) const
{
    for(DebugPoint& point : m_debug_points)
    {
        const std::vector<math::Vector> points = { point.position };
        renderer.DrawPoints(points, point.color, point.size);

        point.timestamp += renderer.GetDeltaTimeMS();
    }

    for(DebugLine& line : m_debug_lines)
    {
        renderer.DrawPolyline(line.points, line.color, line.width);
        line.color.alpha = 1.0f - float(line.timestamp) / 5000.0f;
        line.timestamp += renderer.GetDeltaTimeMS();
    }

    for(DebugText& text : m_debug_texts_world)
    {
        const math::Matrix world_transform = math::CreateMatrixWithPosition(text.position);
        const auto scope = mono::MakeTransformScope(world_transform, &renderer);

        renderer.RenderText(0, text.text.c_str(), text.color, mono::FontCentering::DEFAULT_CENTER);

        text.color.alpha = 1.0f - float(text.timestamp) / 5000.0f;
        text.timestamp += renderer.GetDeltaTimeMS();
    }

    const math::Quad viewport = renderer.GetViewport();
    const math::Matrix projection = math::Ortho(0.0f, math::Width(viewport), 0.0f, math::Height(viewport), -10.0f, 10.0f);

    mono::ScopedTransform projection_scope = mono::MakeProjectionScope(projection, &renderer);
    mono::ScopedTransform view_scope = mono::MakeViewTransformScope(math::Matrix(), &renderer);
    mono::ScopedTransform transform_scope = mono::MakeTransformScope(math::Matrix(), &renderer);

    for(DebugText& text : m_debug_texts_screen)
    {
        const math::Matrix world_transform = math::CreateMatrixWithPosition(text.position);
        const auto scope = mono::MakeTransformScope(world_transform, &renderer);

        renderer.RenderText(0, text.text.c_str(), text.color, mono::FontCentering::DEFAULT_CENTER);

        text.color.alpha = 1.0f - float(text.timestamp) / 5000.0f;
        text.timestamp += renderer.GetDeltaTimeMS();
    }

    mono::remove_if(m_debug_points, remove_if_5<DebugPoint>);
    mono::remove_if(m_debug_lines, remove_if_5<DebugLine>);
    mono::remove_if(m_debug_texts_world, remove_if_5<DebugText>);
    mono::remove_if(m_debug_texts_screen, remove_if_5<DebugText>);
}

math::Quad GameDebugDrawer::BoundingBox() const
{
    return math::InfQuad;
}

void GameDebugDrawer::DrawPoint(const math::Vector& position, float size, const mono::Color::RGBA& color)
{
    const DebugPoint new_point = { position, color, size, 0 };
    m_debug_points.push_back(new_point);
}

void GameDebugDrawer::DrawLine(const math::Vector& start_position, const math::Vector& end_position, float width, const mono::Color::RGBA& color)
{
    DrawLine({start_position, end_position}, width, color);
}

void GameDebugDrawer::DrawLine(const std::vector<math::Vector>& polyline, float width, const mono::Color::RGBA& color)
{
    const DebugLine new_line = { polyline, color, width, 0 };
    m_debug_lines.push_back(new_line);
}

void GameDebugDrawer::DrawScreenText(const char* text, const math::Vector& position, const mono::Color::RGBA& color)
{
    const DebugText new_text = { position, color, 0, text };
    m_debug_texts_screen.push_back(new_text);
}

void GameDebugDrawer::DrawWorldText(const char* text, const math::Vector& position, const mono::Color::RGBA& color)
{
    const DebugText new_text = { position, color, 0, text };
    m_debug_texts_world.push_back(new_text);
}
