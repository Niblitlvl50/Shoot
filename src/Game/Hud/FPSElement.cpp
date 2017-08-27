
#include "FPSElement.h"
#include "FontIds.h"

#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"
#include "Math/Vector.h"

#include <cstdio>

using namespace game;

void FPSElement::Draw(mono::IRenderer& renderer) const
{    
    char text[32] = { '\0' };
    std::snprintf(text, 32, "fps: %u frames: %u", m_counter.Fps(), m_counter.Frames());
    
    constexpr mono::Color::RGBA color(1, 0, 0, 1);
    renderer.DrawText(game::LARGE, text, math::Vector(10, 10), false, color);

    m_counter++;
}
