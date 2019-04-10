
#include "PolygonBoxTool.h"
#include "Editor.h"
#include "RenderLayers.h"
#include "Objects/Polygon.h"
#include "Visualizers/PolygonVisualizer.h"

using namespace editor;

namespace
{
    std::vector<math::Vector> CreatePointsFromPosition(const math::Vector& first, const math::Vector& second)
    {
        const float delta_x = second.x - first.x;
        const float delta_y = second.y - first.y;
        
        return {
            first,
            first + math::Vector(0.0f, delta_y),
            second,
            first + math::Vector(delta_x, 0.0f)
        };
    }
}

PolygonBoxTool::PolygonBoxTool(editor::Editor* editor)
    : m_editor(editor)
{
    m_visualizer = std::make_shared<PolygonVisualizer2>(m_points, m_mouse_position);    
}

void PolygonBoxTool::Begin()
{
    m_editor->AddDrawable(m_visualizer, RenderLayer::OBJECTS);    
}

void PolygonBoxTool::End()
{
    m_editor->RemoveDrawable(m_visualizer);    
}

bool PolygonBoxTool::IsActive() const
{
    return m_active;
}

void PolygonBoxTool::HandleContextMenu(int menu_index)
{ }

void PolygonBoxTool::HandleMouseDown(const math::Vector& world_pos, uint32_t entity_id)
{
    m_active = true;
    m_mouse_down = world_pos;
}

void PolygonBoxTool::HandleMouseUp(const math::Vector& world_pos)
{
    m_active = false;

    const auto& polygon_points = CreatePointsFromPosition(m_mouse_down, world_pos);
    auto polygon = std::make_shared<PolygonEntity>(m_mouse_down, polygon_points);

    m_editor->AddPolygon(polygon);
    m_points.clear();
}

void PolygonBoxTool::HandleMousePosition(const math::Vector& world_pos)
{
    m_mouse_position = world_pos;
    m_points = CreatePointsFromPosition(m_mouse_down, world_pos);
}

void PolygonBoxTool::UpdateModifierState(bool ctrl, bool shift, bool alt)
{ }
