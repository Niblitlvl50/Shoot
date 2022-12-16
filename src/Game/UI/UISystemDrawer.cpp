
#include "UISystemDrawer.h"
#include "UISystem.h"

#include "Math/Matrix.h"
#include "Math/Quad.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"
#include "TransformSystem/TransformSystem.h"
#include "EntitySystem/Entity.h"

using namespace game;

UISystemDrawer::UISystemDrawer(const UISystem* ui_system, mono::TransformSystem* transform_system)
    : m_ui_system(ui_system)
    , m_transform_system(transform_system)
{ }

void UISystemDrawer::Draw(mono::IRenderer& renderer) const
{
    if(!m_ui_system->IsEnabled())
        return;

    const uint32_t active_item = m_ui_system->GetActiveEntityItem();
    if(active_item != mono::INVALID_ID)
    {
        const math::Quad world_bb = m_transform_system->GetWorldBoundingBox(active_item);
        //const auto transform_scope = mono::MakeTransformScope()
        renderer.DrawQuad(math::ResizeQuad(world_bb, 0.05f), mono::Color::CYAN, 1.0f);
    }

    const bool draw_cursor = m_ui_system->DrawCursor();
    if(draw_cursor)
    {
        const math::Vector& cursor_target_position = m_ui_system->GetCursorTargetPosition();
        renderer.DrawPoints({cursor_target_position}, mono::Color::RED, 15.0f);
    }
}

math::Quad UISystemDrawer::BoundingBox() const
{
    return math::InfQuad;
}
