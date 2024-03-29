
#include "GameDebugDrawer.h"
#include "Math/Quad.h"
#include "Math/Matrix.h"
#include "Rendering/IRenderer.h"
#include "Util/Algorithm.h"
#include "FontIds.h"
#include "Rendering/RenderSystem.h"

using namespace game;

game::IDebugDrawer* game::g_debug_drawer = nullptr;

namespace
{
    template <typename T>
    bool remove_if_0(const T& item)
    {
        return item.time_to_live_s < 0.0f;
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

        point.time_to_live_s -= renderer.GetDeltaTime();
    }

    for(DebugLine& line : m_debug_lines)
    {
        renderer.DrawPolyline(line.points, line.color, line.width);
        line.color.alpha = 1.0f - (line.time_to_live_s / 5.0f);
        line.time_to_live_s -= renderer.GetDeltaTime();
    }

    for(DebugText& text : m_debug_texts_world)
    {
        const math::Matrix world_transform = math::CreateMatrixWithPosition(text.position);
        const auto scope = mono::MakeTransformScope(world_transform, &renderer);

        renderer.RenderText(0, text.text.c_str(), text.color, mono::FontCentering::DEFAULT_CENTER);

        text.color.alpha = 1.0f - (text.time_to_live_s / 5.0f);
        text.time_to_live_s -= renderer.GetDeltaTime();
    }


    const float aspect_ratio = mono::RenderSystem::GetWindowAspect();
    const float projection_width = 32.0f;
    const float projection_height = projection_width / aspect_ratio;

    const math::Matrix projection = math::Ortho(0.0f, projection_width, 0.0f, projection_height, 0.0f, 1.0f);

    mono::ScopedTransform projection_scope = mono::MakeProjectionScope(projection, &renderer);
    mono::ScopedTransform view_scope = mono::MakeViewTransformScope(math::Matrix(), &renderer);
    mono::ScopedTransform transform_scope = mono::MakeTransformScope(math::Matrix(), &renderer);

    for(DebugText& text : m_debug_texts_screen)
    {
        const math::Matrix world_transform = math::CreateMatrixWithPosition(text.position);
        const auto scope = mono::MakeTransformScope(world_transform, &renderer);

        renderer.RenderText(FontId::PIXELETTE_TINY, text.text.c_str(), text.color, mono::FontCentering::DEFAULT_CENTER);

        text.color.alpha = 1.0f - (text.time_to_live_s / 5.0f);
        text.time_to_live_s -= renderer.GetDeltaTime();
    }

    mono::remove_if(m_debug_points, remove_if_0<DebugPoint>);
    mono::remove_if(m_debug_lines, remove_if_0<DebugLine>);
    mono::remove_if(m_debug_texts_world, remove_if_0<DebugText>);
    mono::remove_if(m_debug_texts_screen, remove_if_0<DebugText>);
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
    DrawLineFading(polyline, width, color, 0.0f);
}

void GameDebugDrawer::DrawCircle(const math::Vector& position, float radius, const mono::Color::RGBA& color)
{
    DrawCircleFading(position, radius, color, 0.0f);
}

void GameDebugDrawer::DrawScreenText(const char* text, const math::Vector& position, const mono::Color::RGBA& color)
{
    DrawScreenTextFading(text, position, color, 0.0f);
}

void GameDebugDrawer::DrawWorldText(const char* text, const math::Vector& position, const mono::Color::RGBA& color)
{
    const DebugText new_text = { position, color, 0.0f, text };
    m_debug_texts_world.push_back(new_text);
}

void GameDebugDrawer::DrawPointFading(const math::Vector& position, float size, const mono::Color::RGBA& color, float time_s)
{
    const DebugPoint new_point = { position, color, size, time_s };
    m_debug_points.push_back(new_point);
}

void GameDebugDrawer::DrawLineFading(const math::Vector& start_position, const math::Vector& end_position, float width, const mono::Color::RGBA& color, float time_s)
{
    DrawLineFading({ start_position, end_position }, width, color, time_s);
}

void GameDebugDrawer::DrawLineFading(const std::vector<math::Vector>& polyline, float width, const mono::Color::RGBA& color, float time_s)
{
    const DebugLine new_line = { polyline, color, width, time_s };
    m_debug_lines.push_back(new_line);
}

void GameDebugDrawer::DrawCircleFading(const math::Vector& position, float radius, const mono::Color::RGBA& color, float time_s)
{
    constexpr int circle_segments = 32;

    std::vector<math::Vector> vertices;
    vertices.reserve(circle_segments +1);

    const float coef = 2.0f * math::PI() / float(circle_segments);

    for(int index = 0; index < circle_segments +1; ++index)
    {
        const float radians = index * coef;
        const float x = radius * std::cos(radians) + position.x;
        const float y = radius * std::sin(radians) + position.y;

        vertices.emplace_back(x, y);
    }

    DrawLineFading(vertices, 1.0f, color, time_s);
}

void GameDebugDrawer::DrawScreenTextFading(const char* text, const math::Vector& position, const mono::Color::RGBA& color, float time_s)
{
    const DebugText new_text = { position, color, time_s, text };
    m_debug_texts_screen.push_back(new_text);
}
