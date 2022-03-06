
#include "DialogSystemDrawer.h"
#include "DialogSystem.h"
#include "FontIds.h"

#include "Math/Quad.h"
#include "Math/Matrix.h"

#include "Rendering/Color.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Text/TextFunctions.h"

using namespace game;

DialogSystemDrawer::DialogSystemDrawer(DialogSystem* message_system, const mono::TransformSystem* transform_system)
    : m_message_system(message_system)
    , m_transform_system(transform_system)
{ }

void DialogSystemDrawer::Draw(mono::IRenderer& renderer) const
{
    std::vector<std::string> messages;

    const auto collect_messages = [&](uint32_t entity_id, DialogComponent& message) {
        messages.push_back(message.message);
    };
    m_message_system->ForEach(collect_messages);

    if(messages.empty())
        return;

    const math::Matrix new_projection = math::Ortho(0.0f, 12.0f, 0.0f, 8.0f, 0.0f, 1.0f);
    const math::Matrix transform = math::CreateMatrixWithPosition(math::Vector(1.0f, 1.0f));

    const auto transform_scope = mono::MakeTransformScope(transform, &renderer);
    const auto projection_scope = mono::MakeProjectionScope(new_projection, &renderer);
    const auto view_scope = mono::MakeViewTransformScope(math::Matrix(), &renderer);

    constexpr mono::Color::RGBA background_color(0.2f, 0.2f, 0.2f, 0.7f);
    renderer.DrawFilledQuad(math::Quad(math::Vector(-0.1f, -0.1f), math::Vector(8.0f, 1.0f)), background_color);

    for(const std::string& message : messages)
        renderer.RenderText(FontId::RUSSOONE_TINY, message.c_str(), mono::Color::OFF_WHITE, mono::FontCentering::DEFAULT_CENTER);
    //const math::Vector message_size = mono::MeasureString(FontId::RUSSOONE_TINY, message.message.c_str());
}

math::Quad DialogSystemDrawer::BoundingBox() const
{
    return math::InfQuad;
}
