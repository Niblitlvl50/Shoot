
#include "PathTool.h"
#include "Editor.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"
#include "Objects/Path.h"

using namespace editor;

class PathTool::Visualizer : public mono::IDrawable
{
public:

    Visualizer(const std::vector<math::Vector>& points, const math::Vector& mouse_position)
        : m_points(points),
          m_mouse_position(mouse_position)
    { }

    ~Visualizer() = default;

    virtual void doDraw(mono::IRenderer& renderer) const
    {
        if(m_points.empty())
            return;

        constexpr mono::Color::RGBA line_color(1.0f, 0.0f, 0.0f, 0.2f);

        DrawPath(renderer, m_points);

        const std::vector<math::Vector>& line = { m_points.back(), m_mouse_position };
        renderer.DrawLines(line, line_color, 2.0f);
    }

    virtual math::Quad BoundingBox() const
    {
        return math::InfQuad;
    }

    const std::vector<math::Vector>& m_points;
    const math::Vector& m_mouse_position;
};

PathTool::PathTool(Editor* editor)
    : m_editor(editor),
      m_visualizer(std::make_unique<Visualizer>(m_points, m_mouse_position))
{ }

PathTool::~PathTool() = default;

void PathTool::Begin()
{
    m_editor->AddDrawable(m_visualizer.get(), 0);
}

void PathTool::End()
{
    m_editor->RemoveDrawable(m_visualizer.get());
}

bool PathTool::IsActive() const
{
    return true;
}

void PathTool::HandleContextMenu(int menu_index)
{
    if(m_points.empty())
        return;

    if(menu_index == 0)
    {
        const math::Vector position = m_points.front();

        // Make points local
        for(math::Vector& point : m_points)
            point -= position;

        auto path_entity = std::make_unique<editor::PathEntity>("New path", m_points);
        path_entity->SetPosition(position);

        m_editor->AddPath(std::move(path_entity));
        m_points.clear();
    }
    else if(menu_index == 1)
    {
        m_points.pop_back();
    }
}

void PathTool::HandleMouseDown(const math::Vector& world_pos, uint32_t entity_id)
{ }

void PathTool::HandleMouseUp(const math::Vector& world_pos)
{
    m_points.push_back(world_pos);
}

void PathTool::HandleMousePosition(const math::Vector& world_pos)
{
    m_mouse_position = world_pos;
}

void PathTool::UpdateModifierState(bool ctrl, bool shift, bool alt)
{ }
