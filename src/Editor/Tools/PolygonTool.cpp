
#include "PolygonTool.h"
#include "Editor.h"
#include "RenderLayers.h"
#include "Objects/Polygon.h"
#include "Visualizers/PolygonVisualizer.h"


using namespace editor;

PolygonTool::PolygonTool(Editor* editor)
    : m_editor(editor)
{
    m_visualizer = std::make_shared<PolygonVisualizer2>(m_points, m_mouse_position);
}

void PolygonTool::Begin()
{
    m_editor->AddDrawable(m_visualizer, RenderLayer::OBJECTS);
}

void PolygonTool::End()
{
    m_editor->RemoveDrawable(m_visualizer);
}

bool PolygonTool::IsActive() const
{
    return true;
}

void PolygonTool::HandleContextMenu(int menu_index)
{
    if(m_points.empty())
        return;

    if(menu_index == 0)
    {
        m_editor->AddPolygon(std::make_shared<editor::PolygonEntity>(m_points.front(), m_points));
        m_points.clear();
    }
    else if(menu_index == 1)
    {
        m_points.pop_back();
    }
}

void PolygonTool::HandleMouseDown(const math::Vector& world_pos, mono::IEntityPtr entity)
{ }

void PolygonTool::HandleMouseUp(const math::Vector& world_pos)
{
    m_points.push_back(world_pos);
}

void PolygonTool::HandleMousePosition(const math::Vector& world_pos)
{
    m_mouse_position = world_pos;
}
