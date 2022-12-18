
#include "DialogSystemDrawer.h"
#include "DialogSystem.h"
#include "FontIds.h"

#include "Math/Quad.h"
#include "Math/Matrix.h"

#include "Rendering/RenderSystem.h"
#include "Rendering/Color.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Text/TextFunctions.h"
#include "Rendering/RenderBuffer/BufferFactory.h"

using namespace game;

DialogSystemDrawer::DialogSystemDrawer(DialogSystem* message_system, const mono::TransformSystem* transform_system)
    : m_message_system(message_system)
    , m_transform_system(transform_system)
{
    m_texture = mono::GetTextureFactory()->CreateTexture("res/textures/text_background_light.png");
    m_draw_buffers = mono::BuildTextureDrawBuffers(m_texture.get());
}

void DialogSystemDrawer::Draw(mono::IRenderer& renderer) const
{
    std::vector<std::string> messages;

    const auto collect_messages = [&](uint32_t entity_id, DialogComponent& message) {
        messages.push_back(message.message);
    };
    m_message_system->ForEach(collect_messages);

    if(messages.empty())
        return;

    return;

    const float window_aspect = mono::GetWindowAspect();
    const float projection_width = 800.0f;
    const float projection_height = projection_width / window_aspect;
    const float projection_width_half = projection_width / 2.0f;

    const math::Matrix new_projection = math::Ortho(0.0f, projection_width, 0.0f, projection_height, 0.0f, 1.0f);

    const auto projection_scope = mono::MakeProjectionScope(new_projection, &renderer);
    const auto view_scope = mono::MakeViewTransformScope(math::Matrix(), &renderer);

    const math::Matrix bg_transform = math::CreateMatrixWithPosition(math::Vector(projection_width_half, 160.0f));
    const auto bg_transform_scope = mono::MakeTransformScope(bg_transform, &renderer);

    renderer.DrawGeometry(
        m_draw_buffers.vertices.get(),
        m_draw_buffers.uv.get(),
        m_draw_buffers.indices.get(),
        m_texture.get(),
        mono::Color::WHITE,
        false,
        m_draw_buffers.indices->Size());

/*
    {
        const math::Matrix name_transform = bg_transform * math::CreateMatrixWithPosition(math::Vector(-200.0f, 42.0f));
        const auto name_transform_scope = mono::MakeTransformScope(name_transform, &renderer);
        renderer.RenderText(FontId::BLACK_HAN_SANS_MEDIUM, "Niklas", mono::Color::GRAY, mono::FontCentering::DEFAULT_CENTER);
    }

    {
        const math::Matrix text_transform = bg_transform * math::CreateMatrixWithPosition(math::Vector(-200.0f, 0.0f));
        const auto text_transform_scope = mono::MakeTransformScope(text_transform, &renderer);

        for(const std::string& message : messages)
            renderer.RenderText(FontId::BLACK_HAN_SANS_MEDIUM, message.c_str(), mono::Color::OFF_WHITE, mono::FontCentering::DEFAULT_CENTER);
    }
*/

    //const math::Vector message_size = mono::MeasureString(FontId::RUSSOONE_TINY, message.message.c_str());
}

math::Quad DialogSystemDrawer::BoundingBox() const
{
    return math::InfQuad;
}
