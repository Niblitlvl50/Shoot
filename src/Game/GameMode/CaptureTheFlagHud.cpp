
#include "CaptureTheFlagHud.h"

#include "Math/Matrix.h"
#include "Math/Quad.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"
#include "FontIds.h"

#include <cstdio>

using namespace game;

CaptureTheFlagHud::CaptureTheFlagHud(const CaptureTheFlagScore& score)
    : m_score(score)
{ }

void CaptureTheFlagHud::Draw(mono::IRenderer& renderer) const
{
    const math::Matrix& projection = math::Ortho(0.0f, 200.0f, 0.0f, 100.0f, -10.0f, 10.0f);

    const mono::ScopedTransform transform_scope = mono::MakeTransformScope(math::Matrix(), &renderer);
    const mono::ScopedTransform projection_scope = mono::MakeProjectionScope(projection, &renderer);

    char text_buffer[64] = { 0 };
    std::sprintf(text_buffer, "Red %d | Blue %d", m_score.red, m_score.blue);

    renderer.DrawText(FontId::PIXELETTE_LARGE, text_buffer, math::Vector(100.0f, 90.0f), true, mono::Color::BLUE);
}

math::Quad CaptureTheFlagHud::BoundingBox() const
{
    return math::InfQuad;
}
