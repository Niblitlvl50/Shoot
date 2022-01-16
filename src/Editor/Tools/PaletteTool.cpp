
#include "PaletteTool.h"
#include "Editor.h"
#include "UI/UIProperties.h"
#include "RenderLayers.h"

#include "Rendering/IDrawable.h"
#include "Rendering/IRenderer.h"
#include "Util/Random.h"
#include "Util/StringFunctions.h"

#include "Entity/Component.h"

using namespace editor;

PaletteTool::PaletteTool(Editor* editor)
    : m_editor(editor)
    , m_active_index(0)
{
    m_colors = {
        mono::Color::MakeFromBytes(38, 70, 83),
        mono::Color::MakeFromBytes(42, 157, 143),
        mono::Color::MakeFromBytes(233, 196, 106),
        mono::Color::MakeFromBytes(244, 162, 97),
        mono::Color::MakeFromBytes(231, 111, 81),
    };
}

void PaletteTool::Begin()
{
    m_editor->AddDrawable(this, RenderLayer::UI);
}

void PaletteTool::End()
{
    m_editor->RemoveDrawable(this);
}

bool PaletteTool::IsActive() const
{
    return false;
}

void PaletteTool::HandleContextMenu(int menu_index)
{ }

void PaletteTool::HandleMouseDown(const math::Vector& world_pos, uint32_t entity_id)
{ }

void PaletteTool::HandleMouseUp(const math::Vector& world_pos)
{ }

void PaletteTool::HandleMousePosition(const math::Vector& world_pos)
{ }

void PaletteTool::UpdateModifierState(bool ctrl, bool shift, bool alt)
{ }

void PaletteTool::Draw(mono::IRenderer& renderer) const
{
    const PaletteResult result = DrawPaletteView(m_colors, m_active_index);
    m_active_index = result.selected_index;

    if(result.apply || result.reset)
    {
        const std::vector<uint32_t> selection = m_editor->GetSelection();
        for(uint32_t id : selection)
        {
            IObjectProxy* proxy = m_editor->FindProxyObject(id);
            std::vector<Component>& components = proxy->GetComponents();

            Component* sprite_component = FindComponentFromHash(SPRITE_COMPONENT, components);
            if(sprite_component)
            {
                const mono::Color::RGBA new_color = (result.apply) ? m_colors[mono::RandomInt(0, m_colors.size() -1)] : mono::Color::WHITE;
                SetAttribute(COLOR_ATTRIBUTE, sprite_component->properties, new_color);
                proxy->ComponentChanged(*sprite_component, COLOR_ATTRIBUTE);
            }
        }
    }

    if(!result.pasted_text.empty())
    {
        m_colors.clear();

        const std::vector<std::string> tokens = mono::SplitString(result.pasted_text, ',');
        for(const std::string& hex : tokens)
        {
            const uint32_t hex_value = std::stoul(hex + "ff", nullptr, 16);
            m_colors.push_back(mono::Color::ToRGBA(hex_value));
        }
    }
}

math::Quad PaletteTool::BoundingBox() const
{
    return math::InfQuad;
}
