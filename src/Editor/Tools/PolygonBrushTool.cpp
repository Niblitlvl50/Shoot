
#include "PolygonBrushTool.h"
#include "Editor.h"
#include "RenderLayers.h"
#include "Objects/Polygon.h"

#include "Math/MathFunctions.h"
#include "Math/Quad.h"

#include "Rendering/IDrawable.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"

#include <cmath>

using namespace editor;

class PolygonBrushTool::Visualizer : public mono::IDrawable
{
public:

    Visualizer(const bool& painting, const std::vector<math::Vector>& drawn_points)
        : m_painting(painting),
          m_points(drawn_points)
    { }

    virtual void doDraw(mono::IRenderer& renderer) const
    {
        if(!m_painting)
            return;

        constexpr mono::Color::RGBA color(0.0f, 1.0f, 0.5f, 1.0f);
        renderer.DrawPoints(m_points, color, 2.0f);
    }

    virtual math::Quad BoundingBox() const
    {
        return math::InfQuad;
    }

    const bool& m_painting;
    const std::vector<math::Vector>& m_points;
};

PolygonBrushTool::PolygonBrushTool(Editor* editor)
    : m_editor(editor),
      m_painting(false)
{
    m_visualizer = std::make_shared<PolygonBrushTool::Visualizer>(m_painting, m_drawn_points);
}

void PolygonBrushTool::Begin()
{ }

void PolygonBrushTool::End()
{ }

bool PolygonBrushTool::IsActive() const
{
    return m_painting;
}

void PolygonBrushTool::HandleContextMenu(int menu_index)
{ }

void PolygonBrushTool::HandleMouseDown(const math::Vector& world_pos, uint32_t entity_id)
{
    m_painting = true;
    m_direction = math::INF;
    m_previous_point = world_pos;
    m_previously_added_point = world_pos;

    m_polygon = std::make_shared<editor::PolygonEntity>();
    m_polygon->SetPosition(world_pos);
    m_polygon->AddVertex(math::ZeroVec);

    m_editor->AddPolygon(m_polygon);
    m_editor->AddDrawable(m_visualizer, RenderLayer::OBJECTS);
}

void PolygonBrushTool::HandleMouseUp(const math::Vector& world_pos)
{
    m_editor->RemoveDrawable(m_visualizer);

    m_painting = false;
    m_polygon = nullptr;
    m_drawn_points.clear();
}

void PolygonBrushTool::HandleMousePosition(const math::Vector& world_pos)
{
    if(!m_painting)
        return;

    if(m_direction == math::INF)
    {
        m_direction = math::AngleBetweenPoints(m_previously_added_point, world_pos);
        return;
    }

    constexpr float threashold = math::ToRadians(5.0f);
    const float angle = std::fabs(m_direction - math::AngleBetweenPoints(m_previously_added_point, world_pos));

    const math::Vector& position_diff = world_pos - m_previously_added_point;
    const float distance = math::Length(position_diff);

    if(angle > threashold && distance > 1.0f)
    {
        const math::Vector& position = m_polygon->Position();
        m_polygon->AddVertex(m_previous_point - position);

        m_previously_added_point = m_previous_point;
        m_direction = math::AngleBetweenPoints(m_previously_added_point, world_pos);
    }

    m_previous_point = world_pos;
    m_drawn_points.push_back(m_previous_point);
}

void PolygonBrushTool::UpdateModifierState(bool ctrl, bool shift, bool alt)
{ }
