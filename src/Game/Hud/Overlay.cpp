
#include "Overlay.h"
#include "Math/Quad.h"
#include "Rendering/IRenderer.h"

using namespace game;

UIOverlayDrawer::UIOverlayDrawer()
{
    //m_projection = math::Ortho(0.0f, 300.0f, 0.0f, 200.0f, -10.0f, 10.0f);
    m_projection = math::Ortho(0.0f, 200.0f, 0.0f, 300.0f, -10.0f, 10.0f);
}

void UIOverlayDrawer::Draw(mono::IRenderer& renderer) const
{
    const mono::ScopedTransform transform_scope = mono::MakeTransformScope(math::Matrix(), &renderer);
    const mono::ScopedTransform projection_scope = mono::MakeProjectionScope(m_projection, &renderer);

    EntityBase::Draw(renderer);
}

void UIOverlayDrawer::EntityDraw(mono::IRenderer& renderer) const
{ }

void UIOverlayDrawer::EntityUpdate(const mono::UpdateContext& update_context)
{ }

math::Quad UIOverlayDrawer::BoundingBox() const
{
    return math::InfQuad;
}
