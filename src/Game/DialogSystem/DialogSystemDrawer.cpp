
#include "DialogSystemDrawer.h"
#include "DialogSystem.h"
#include "FontIds.h"

#include "Math/Quad.h"
#include "Math/Matrix.h"

#include "Rendering/Color.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Text/TextFunctions.h"

using namespace game;

DialogSystemDrawer::DialogSystemDrawer(DialogSystem* message_system)
    : m_message_system(message_system)
{ }

void DialogSystemDrawer::Draw(mono::IRenderer& renderer) const
{
    const math::Matrix new_projection = math::Ortho(0.0f, 12.0f, 0.0f, 8.0f, 0.0f, 1.0f);

    const auto projection_scope = mono::MakeProjectionScope(new_projection, &renderer);
    const auto view_scope = mono::MakeViewTransformScope(math::Matrix(), &renderer);

    const auto collect_messages = [&](uint32_t entity_id, DialogComponent& message) {
        const math::Vector message_size = mono::MeasureString(FontId::PIXELETTE_SMALL, message.message.c_str());
        renderer.DrawFilledQuad(math::Quad(math::ZeroVec, message_size), mono::Color::CYAN);
        renderer.RenderText(FontId::PIXELETTE_SMALL, message.message.c_str(), mono::Color::RED, mono::FontCentering::DEFAULT_CENTER);
    };
    m_message_system->ForEach(collect_messages);
}

math::Quad DialogSystemDrawer::BoundingBox() const
{
    return math::InfQuad;
}
