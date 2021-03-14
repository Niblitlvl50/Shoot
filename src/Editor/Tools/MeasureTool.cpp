
#include "MeasureTool.h"
#include "Rendering/IDrawable.h"
#include "Rendering/IRenderer.h"
#include "Math/Quad.h"
#include "Math/Matrix.h"

#include "Editor.h"
#include "RenderLayers.h"
#include "FontIds.h"

#include <cmath>
#include <cstdio>

using namespace editor;

class MeasureTool::Visualizer : public mono::IDrawable
{
public:

    Visualizer(const bool& active, const math::Vector& start_position, const math::Vector& end_position)
        : m_active(active)
        , m_start_position(start_position)
        , m_end_position(end_position)
    { }

    void Draw(mono::IRenderer& renderer) const override
    {
        if(!m_active)
            return;

        const std::vector<math::Vector> measure_line = {
            m_start_position,
            m_end_position
        };

        const math::Vector delta = m_start_position - m_end_position;
        const float radius = std::fabs(math::Length(delta));

        const mono::Color::RGBA circle_color(1.0f, 0.0f, 1.0f, 0.5f);

        renderer.DrawPoints(measure_line, mono::Color::BLUE, 3.0f);
        renderer.DrawLines(measure_line, mono::Color::BLUE, 1.5f);
        renderer.DrawCircle(m_start_position, radius, 30, 1.0f, mono::Color::BLUE);

        const math::Vector text_position = m_start_position - math::Vector(0.0f, 0.5f);

        const math::Matrix text_transform = math::CreateMatrixWithPosition(text_position);
        const auto text_scope = mono::MakeTransformScope(text_transform, &renderer);

        char text_buffer[512] = { 0 };
        std::sprintf(text_buffer, "%.2f", radius);
        renderer.RenderText(
            shared::FontId::PIXELETTE_SMALL,
            text_buffer,
            mono::Color::BLUE,
            mono::FontCentering::HORIZONTAL_VERTICAL);
    }

    math::Quad BoundingBox() const override
    {
        return math::InfQuad;
    }

    const bool& m_active;
    const math::Vector& m_start_position;
    const math::Vector& m_end_position;
};

MeasureTool::MeasureTool(Editor* editor)
    : m_editor(editor)
    , m_active(false)
    , m_visualizer(std::make_unique<Visualizer>(m_active, m_start_position, m_end_position))
{ }

MeasureTool::~MeasureTool()
{ }

void MeasureTool::Begin()
{
    m_editor->AddDrawable(m_visualizer.get(), RenderLayer::UI);
}

void MeasureTool::End()
{
    m_active = false;
    m_editor->RemoveDrawable(m_visualizer.get());
}

bool MeasureTool::IsActive() const
{
    return true;
}

void MeasureTool::HandleContextMenu(int menu_index)
{ }

void MeasureTool::HandleMouseDown(const math::Vector& world_pos, uint32_t entity_id)
{ }

void MeasureTool::HandleMouseUp(const math::Vector& world_pos)
{ }

void MeasureTool::HandleMousePosition(const math::Vector& world_pos)
{
    if(!m_active)
        m_start_position = world_pos;

    m_end_position = world_pos;
    m_active = true;
}

void MeasureTool::UpdateModifierState(bool ctrl, bool shift, bool alt)
{ }
