
#include "UISystemDrawer.h"
#include "UISystem.h"

#include "Math/Matrix.h"
#include "Math/Quad.h"
#include "Rendering/IRenderer.h"
#include "TransformSystem/TransformSystem.h"

using namespace game;

UISystemDrawer::UISystemDrawer(const UISystem* ui_system, mono::TransformSystem* transform_system)
    : m_ui_system(ui_system)
    , m_transform_system(transform_system)
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
    }

    const std::vector<uint32_t>& active_items = m_ui_system->GetActiveItems();
    for(uint32_t item_id : active_items)
    {
        const math::Quad world_bb = m_transform_system->GetWorldBoundingBox(item_id);
        //const auto transform_scope = mono::MakeTransformScope()
        renderer.DrawQuad(world_bb, mono::Color::MAGENTA, 1.0f);
    }
}

math::Quad UISystemDrawer::BoundingBox() const
{
    return math::InfQuad;
}
