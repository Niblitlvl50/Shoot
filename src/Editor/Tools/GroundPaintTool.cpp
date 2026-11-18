
#include "GroundPaintTool.h"
#include "Editor.h"
#include "RenderLayers.h"
#include "UI/UIContext.h"
#include "World/GroundPaint.h"

#include "Math/Quad.h"
#include "Rendering/IDrawable.h"
#include "Rendering/IRenderer.h"

#include <string>

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
    , m_pick(false)
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
    if(m_pick)
    {
        const int picked_layer = m_editor->GetGroundPaintData()->StrongestLayer(world_pos);
        if(picked_layer == 0)
            m_context->notifications.emplace_back(editor::wrench_texture, "Base layer, erase with shift to paint it", 1.0f);
        else
            SelectLayer(picked_layer);
        return;
    }

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
    m_erase = shift;
    m_pick = alt;
}

void GroundPaintTool::SelectLayer(int layer)
{
    m_context->ground_paint_brush.layer = layer;

    // Just the file name of the texture, without folders and extension.
    const std::string& texture_file = m_context->level_metadata.ground_paint.textures[layer];
    const size_t name_start = texture_file.find_last_of("/\\") + 1;
    const size_t extension_start = texture_file.find_last_of('.');
    const size_t name_end = (extension_start != std::string::npos && extension_start > name_start) ? extension_start : texture_file.size();
    const std::string texture_name = texture_file.empty() ? "no texture" : texture_file.substr(name_start, name_end - name_start);

    const std::string text = "Layer " + std::to_string(layer) + ": " + texture_name;
    m_context->notifications.emplace_back(editor::wrench_texture, text.c_str(), 1.0f);
}

void GroundPaintTool::PaintAt(const math::Vector& world_pos)
{
    m_brush_position = world_pos;

    const UIContext::GroundPaintBrush& brush = m_context->ground_paint_brush;
    const int layer = m_erase ? 0 : brush.layer;
    m_editor->GetGroundPaintData()->Paint(world_pos, brush.radius, brush.strength, layer);
}
