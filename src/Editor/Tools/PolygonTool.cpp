
#include "PolygonTool.h"
#include "Editor.h"
#include "RenderLayers.h"
#include "Objects/Polygon.h"
#include "Visualizers/PolygonVisualizer.h"


using namespace editor;

PolygonTool::PolygonTool(Editor* editor)
    : m_editor(editor)
{
    m_visualizer = std::make_unique<PolygonVisualizer2>(m_points, m_mouse_position);
}

PolygonTool::~PolygonTool() = default;

void PolygonTool::Begin()
{
    m_editor->AddDrawable(m_visualizer.get(), RenderLayer::OBJECTS);
}

void PolygonTool::End()
{
    m_editor->RemoveDrawable(m_visualizer.get());
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
        m_editor->AddPolygon(std::make_unique<editor::PolygonEntity>(m_points.front(), m_points));
        m_points.clear();
    }
    else if(menu_index == 1)
    {
        m_points.pop_back();
    }
}

void PolygonTool::HandleMouseDown(const math::Vector& world_pos, uint32_t entity_id)
{ }

void PolygonTool::HandleMouseUp(const math::Vector& world_pos)
{
    m_points.push_back(world_pos);
}

void PolygonTool::HandleMousePosition(const math::Vector& world_pos)
{
    m_mouse_position = world_pos;
}

void PolygonTool::UpdateModifierState(bool ctrl, bool shift, bool alt)
{ }
