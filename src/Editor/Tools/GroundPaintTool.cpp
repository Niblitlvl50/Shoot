
#include "GroundPaintTool.h"
#include "Editor.h"
#include "RenderLayers.h"
#include "UI/UIContext.h"
#include "World/GroundPaint.h"

#include "Math/Quad.h"
#include "Rendering/IDrawable.h"
#include "Rendering/IRenderer.h"

using namespace editor;

class GroundPaintTool::Visualizer : public mono::IDrawable
{
public:

    Visualizer(const math::Vector& brush_position, const bool& erase, const UIContext* context)
        : m_brush_position(brush_position)
        , m_erase(erase)
        , m_context(context)
    { }

    void Draw(mono::IRenderer& renderer) const override
    {
        const game::GroundPaintSettings& settings = m_context->level_metadata.ground_paint;
        const math::Quad area = math::Quad(settings.position, settings.position + settings.size);
        renderer.DrawQuad(area, mono::Color::OFF_WHITE, 1.0f);

        const mono::Color::RGBA& brush_color = m_erase ? mono::Color::RED : mono::Color::GREEN;
        renderer.DrawCircle(m_brush_position, m_context->ground_paint_brush.radius, 32, 1.0f, brush_color);
    }

    math::Quad BoundingBox() const override
    {
        return math::InfQuad;
    }

    const math::Vector& m_brush_position;
    const bool& m_erase;
    const UIContext* m_context;
};

GroundPaintTool::GroundPaintTool(Editor* editor, UIContext* context)
    : m_editor(editor)
    , m_context(context)
    , m_painting(false)
    , m_erase(false)
    , m_visualizer(std::make_unique<Visualizer>(m_brush_position, m_erase, context))
{ }

GroundPaintTool::~GroundPaintTool()
{ }

void GroundPaintTool::Begin()
{
    m_editor->AddDrawable(m_visualizer.get(), game::LayerId::UI);
}

void GroundPaintTool::End()
{
    m_painting = false;
    m_editor->RemoveDrawable(m_visualizer.get());
}

bool GroundPaintTool::IsActive() const
{
    return true;
}

void GroundPaintTool::HandleContextMenu(int menu_index)
{ }

void GroundPaintTool::HandleMouseDown(const math::Vector& world_pos, uint32_t entity_id)
{
    m_painting = true;
    PaintAt(world_pos);
}

void GroundPaintTool::HandleMouseUp(const math::Vector& world_pos)
{
    m_painting = false;
}

void GroundPaintTool::HandleMousePosition(const math::Vector& world_pos)
{
    m_brush_position = world_pos;
    if(m_painting)
        PaintAt(world_pos);
}

void GroundPaintTool::UpdateModifierState(bool ctrl, bool shift, bool alt)
{
    m_erase = alt;
}

void GroundPaintTool::PaintAt(const math::Vector& world_pos)
{
    m_brush_position = world_pos;

    const UIContext::GroundPaintBrush& brush = m_context->ground_paint_brush;
    const int layer = m_erase ? 0 : brush.layer;
    m_editor->GetGroundPaintData()->Paint(world_pos, brush.radius, brush.strength, layer);
}
