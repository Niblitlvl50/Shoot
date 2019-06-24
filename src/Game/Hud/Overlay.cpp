
#include "Overlay.h"
#include "Math/Quad.h"
#include "Rendering/IRenderer.h"
#include "System/UID.h"

using namespace game;

UIOverlayDrawer::UIOverlayDrawer()
{
    m_projection = math::Ortho(0.0f, 300.0f, 0.0f, 200.0f, -10.0f, 10.0f);
}

void UIOverlayDrawer::doDraw(mono::IRenderer& renderer) const
{
    constexpr math::Matrix identity;
    renderer.PushNewTransform(identity);
    renderer.PushNewProjection(m_projection);

    EntityBase::doDraw(renderer);
}

void UIOverlayDrawer::Draw(mono::IRenderer& renderer) const
{ }

void UIOverlayDrawer::Update(const mono::UpdateContext& update_context)
{ }

math::Quad UIOverlayDrawer::BoundingBox() const
{
    return math::InfQuad;
}
