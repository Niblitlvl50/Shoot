
#include "Background.h"
#include "Rendering/IRenderer.h"

using namespace game;

Background::Background(const math::Quad& viewport, const mono::Color::HSL& color)
    : m_viewport(viewport)
    , m_color(color)
{ }

void Background::EntityDraw(mono::IRenderer& renderer) const
{
    renderer.DrawFilledQuad(m_viewport, mono::Color::ToRGBA(m_color));
}

void Background::EntityUpdate(const mono::UpdateContext& update_context)
{ }
