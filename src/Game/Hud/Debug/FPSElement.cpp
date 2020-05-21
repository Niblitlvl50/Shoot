
#include "FPSElement.h"
#include "FontIds.h"

#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"
#include "Math/Vector.h"

#include "GameDebug.h"

#include <cstdio>

using namespace game;

FPSElement::FPSElement(const math::Vector& position)
    : FPSElement(position, mono::Color::RGBA(1, 0, 0))
{ }

FPSElement::FPSElement(const math::Vector& position, const mono::Color::RGBA& color)
    : m_color(color)
{
    m_position = position;
}

void FPSElement::EntityDraw(mono::IRenderer& renderer) const
{
    if(!game::g_draw_fps)
        return;

    char text[32] = { '\0' };
    std::snprintf(text, 32, "fps: %u frames: %u", m_counter.Fps(), m_counter.Frames());
    renderer.DrawText(game::PIXELETTE_MEGA, text, math::ZeroVec, false, m_color);
}

void FPSElement::EntityUpdate(const mono::UpdateContext& update_context)
{
    m_counter++;
}

