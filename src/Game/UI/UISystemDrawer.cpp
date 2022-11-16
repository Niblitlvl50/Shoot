
#include "UISystemDrawer.h"
#include "UISystem.h"

#include "Rendering/IRenderer.h"
#include "Math/Matrix.h"
#include "Math/Quad.h"

using namespace game;

UISystemDrawer::UISystemDrawer(const class UISystem* ui_system)
    : m_ui_system(ui_system)
{ }

void UISystemDrawer::Draw(mono::IRenderer& renderer) const
{
    const std::vector<UILayer>& layers = m_ui_system->GetLayers();
    for(const UILayer& layer : layers)
    {
        if(!layer.enabled)
            continue;

        const math::Matrix projection = math::Ortho(0, layer.width, 0, layer.height, -10, 10);
        const auto projection_scope = mono::MakeProjectionScope(projection, &renderer);
        const auto view_scope = mono::MakeViewTransformScope(math::Matrix(), &renderer);

        const math::Vector layer_size = math::Vector(layer.width, layer.height);

        for(const uint32_t text_item_id : layer.text_items)
        {
            const UITextItem* text_item = m_ui_system->FindTextItem(text_item_id);
            if(!text_item)
                continue;

            const math::Vector fraction_position = text_item->position * layer_size;

            const math::Matrix text_transform = math::CreateMatrixWithPosition(fraction_position);
            const auto transform_scope = mono::MakeTransformScope(text_transform, &renderer);
            renderer.RenderText(text_item->font_id, text_item->text.c_str(), text_item->color, text_item->centering);
        }
    }
}

math::Quad UISystemDrawer::BoundingBox() const
{
    return math::InfQuad;
}
