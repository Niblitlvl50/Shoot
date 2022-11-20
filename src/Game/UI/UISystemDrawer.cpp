
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

            const math::Quad text_quad = math::Quad(text_item->text_offset, text_item->text_size + text_item->text_offset);
            renderer.DrawQuad(text_quad, mono::Color::BLACK, 1.0f);
        }

        for(const uint32_t rect_item_id : layer.rect_items)
        {
            const UIRectangle* rect_item = m_ui_system->FindRectItem(rect_item_id);
            if(!rect_item)
                continue;

            const math::Vector fraction_position = rect_item->position * layer_size;

            const math::Matrix text_transform = math::CreateMatrixWithPosition(fraction_position);
            const auto transform_scope = mono::MakeTransformScope(text_transform, &renderer);


            //renderer.DrawTrianges(m_vertices.get(), m_colors.get(), m_indices.get(), 0, 6);

            const math::Quad quad = math::Quad(math::ZeroVec, rect_item->size.x, rect_item->size.y);
            renderer.DrawFilledQuad(quad, rect_item->color);
            //renderer.RenderText(text_item->font_id, text_item->text.c_str(), text_item->color, text_item->centering);
            if(rect_item->border_width > 0.0f)
            {
                //renderer.DrawPolyline(m_vertices.get(), m_border_colors.get(), m_indices.get(), 6, 5);

            }
        }
    }
}

math::Quad UISystemDrawer::BoundingBox() const
{
    return math::InfQuad;
}
