
#include "InteractionSystemDrawer.h"
#include "InteractionSystem.h"

#include "Math/Quad.h"
#include "Math/Matrix.h"

#include "Rendering/Color.h"
#include "Rendering/IRenderer.h"
#include "Rendering/RenderSystem.h"
#include "Rendering/RenderBuffer/IRenderBuffer.h"
#include "Rendering/RenderBuffer/BufferFactory.h"
#include "Rendering/Sprite/ISpriteFactory.h"
#include "Rendering/Sprite/ISprite.h"
#include "Rendering/Sprite/Sprite.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Text/TextFunctions.h"

#include "EntitySystem/EntitySystem.h"
#include "TransformSystem/TransformSystem.h"

#include "FontIds.h"

namespace
{
    constexpr const char* g_interaction_type_sprite[] = {
        "res/sprites/button_cross.sprite",
        "res/sprites/button_cross.sprite",
        "res/sprites/button_cross.sprite",
        "res/sprites/button_cross.sprite",
    };

    static_assert(std::size(g_interaction_type_sprite) == std::size(game::interaction_type_verb));
}

namespace tweak_values
{
    constexpr game::FontId verb_font = game::FontId::RUSSOONE_TINY;
    constexpr mono::Color::RGBA verb_color = mono::Color::OFF_WHITE;
    constexpr mono::Color::RGBA background_color = mono::Color::RGBA(0.4f, 0.4f, 0.4f, 0.5f);
}

using namespace game;

InteractionSystemDrawer::InteractionSystemDrawer(
    InteractionSystem* interaction_system,
    mono::SpriteSystem* sprite_system,
    const mono::TransformSystem* transform_system,
    const mono::EntitySystem* entity_system)
    : m_interaction_system(interaction_system)
    , m_sprite_system(sprite_system)
    , m_transform_system(transform_system)
    , m_entity_system(entity_system)
{
    for(uint32_t index = 0; index < std::size(interaction_type_verb); ++index)
    {
        const char* verb = interaction_type_verb[index];

        m_verb_text_buffers.push_back(mono::BuildTextDrawBuffers(tweak_values::verb_font, verb, mono::FontCentering::VERTICAL));

        const mono::TextMeasurement text_measurement = mono::MeasureString(tweak_values::verb_font, verb);
        m_verb_text_widths.push_back(text_measurement.size.x);

        VerbSpriteBuffer verb_sprite_buffer;
        verb_sprite_buffer.sprite = mono::RenderSystem::GetSpriteFactory()->CreateSprite(g_interaction_type_sprite[index]);
        verb_sprite_buffer.sprite_buffer = mono::BuildSpriteDrawBuffers(verb_sprite_buffer.sprite->GetSpriteData());

        m_verb_sprites_buffers.push_back(std::move(verb_sprite_buffer));
    }

    for(const char* verb : interaction_type_verb)
    {
        m_verb_text_buffers.push_back(mono::BuildTextDrawBuffers(tweak_values::verb_font, verb, mono::FontCentering::VERTICAL));

        const mono::TextMeasurement text_measurement = mono::MeasureString(tweak_values::verb_font, verb);
        m_verb_text_widths.push_back(text_measurement.size.x);
    }

    constexpr uint16_t indices[] = {
        0, 1, 2, 0, 2, 3
    };
    m_indices = mono::CreateElementBuffer(mono::BufferType::STATIC, 6, indices);
}

void InteractionSystemDrawer::Draw(mono::IRenderer& renderer) const
{
    // Dirty fix for updating the interaction sprites.
    {
        mono::UpdateContext update_context;
        update_context.delta_s = renderer.GetDeltaTime();
        update_context.timestamp = renderer.GetTimestamp();
        update_context.paused = false;

        for(VerbSpriteBuffer& verb_sprite : m_verb_sprites_buffers)
            verb_sprite.sprite->Update(update_context);
    }

    struct InteractionDrawData
    {
        math::Matrix transform;
        uint32_t sprite_index;
        const char* name;
    };
    std::vector<InteractionDrawData> active_interactions;

    const FrameInteractionData& frame_interactions = m_interaction_system->GetFrameInteractionData();

    for(const InteractionAndTrigger& interaction_trigger : frame_interactions.active)
    {
        const bool has_sprite = m_sprite_system->IsAllocated(interaction_trigger.interaction_id);
        const auto interaction_it = m_backup_data.find(interaction_trigger.interaction_id);
        if(has_sprite && interaction_it == m_backup_data.end())
        {
            mono::Sprite* sprite = m_sprite_system->GetSprite(interaction_trigger.interaction_id);
            mono::Color::RGBA shade = sprite->GetShade();
            
            mono::Color::HSL hsl_shade = mono::Color::ToHSL(shade);
            hsl_shade.lightness *= 1.2f;

            sprite->SetShade(mono::Color::ToRGBA(hsl_shade, shade.alpha));
            m_backup_data[interaction_trigger.interaction_id] = { shade };
        }

        const math::Quad& entity_world_bb = m_transform_system->GetWorldBoundingBox(interaction_trigger.interaction_id);
        const math::Vector& entity_position = math::TopCenter(entity_world_bb) + math::Vector(0.0f, 0.5f);

        const math::Matrix transform = math::CreateMatrixWithPosition(entity_position);

        const char* entity_name =
            interaction_trigger.draw_name ? m_entity_system->GetEntityName(interaction_trigger.interaction_id) : nullptr;
        active_interactions.push_back({ transform, (uint32_t)interaction_trigger.interaction_type, entity_name });
    }

    for(const InteractionAndTrigger& interaction_trigger : frame_interactions.deactivated)
    {
        const auto it = m_backup_data.find(interaction_trigger.interaction_id);
        if(it == m_backup_data.end())
            continue;

        mono::Sprite* sprite = m_sprite_system->GetSprite(interaction_trigger.interaction_id);
        sprite->SetShade(it->second.shade);

        m_backup_data.erase(interaction_trigger.interaction_id);
    }

    for(const InteractionDrawData& draw_data : active_interactions)
    {
        // Name
        if(draw_data.name != nullptr)
        {
            const auto scope = mono::MakeTransformScope(draw_data.transform, &renderer);

            const float padding = 0.1f;

            const mono::TextMeasurement text_measurement = mono::MeasureString(tweak_values::verb_font, draw_data.name);
            const math::Vector half_text_size = (text_measurement.size / 2.0f) + math::Vector(padding, padding);

            const math::Quad background_quad(-half_text_size, half_text_size);
            renderer.DrawFilledQuad(background_quad, tweak_values::background_color);
            //renderer.DrawQuad(background_quad, mono::Color::BLACK, 1.0f);

            renderer.RenderText(
                tweak_values::verb_font,
                draw_data.name,
                tweak_values::verb_color,
                mono::FontCentering::HORIZONTAL_VERTICAL);
        }

        // Verb
        {
            /*
            const math::Matrix projection = math::Ortho(0.0f, 12.0f, 0.0f, 8.0f, 0.0f, 1.0f);
            const math::Matrix transform = math::CreateMatrixWithPosition(math::Vector(10.0f, 1.5f));

            const auto projection_scope = mono::MakeProjectionScope(projection, &renderer);
            const auto transform_scope = mono::MakeTransformScope(transform, &renderer);
            const auto view_scope = mono::MakeViewTransformScope(math::Matrix(), &renderer);
            */

            const float verb_width = m_verb_text_widths[draw_data.sprite_index];
            const float width_padding = 0.1f;

            const VerbSpriteBuffer& sprite_and_buffer = m_verb_sprites_buffers[draw_data.sprite_index];
            const mono::ISprite* button_sprite = sprite_and_buffer.sprite.get();
            const mono::SpriteDrawBuffers& button_buffers = sprite_and_buffer.sprite_buffer;

            const mono::SpriteFrame current_frame = button_sprite->GetCurrentFrame();
            const float half_sprite_width = current_frame.size.x / 2.0f;

            const math::Quad background_quad(
                -(half_sprite_width + width_padding), -0.25f,
                (current_frame.size.x + verb_width + width_padding), 0.25f);

            const float quad_width = math::Width(background_quad);

            math::Matrix transform = draw_data.transform;
            math::Translate(transform, math::Vector(-(quad_width * 0.25f), 0.0f));
            math::ScaleXY(transform, math::Vector(0.75f, 0.75f));

            const auto scope = mono::MakeTransformScope(transform, &renderer);
            renderer.DrawFilledQuad(background_quad, tweak_values::background_color);

            const int offset = sprite_and_buffer.sprite->GetCurrentFrameIndex() * button_buffers.vertices_per_sprite;
            renderer.DrawSprite(button_sprite, &button_buffers, m_indices.get(), offset);

            const mono::TextDrawBuffers& text_buffers = m_verb_text_buffers[draw_data.sprite_index];
            const mono::ITexturePtr font_texture = mono::GetFontTexture(tweak_values::verb_font);

            math::Matrix text_transform = transform;
            math::Translate(text_transform, math::Vector(0.25f, 0.0f));
            const auto text_scope = mono::MakeTransformScope(text_transform, &renderer);

            renderer.DrawGeometry(
                text_buffers.vertices.get(),
                text_buffers.uv.get(),
                text_buffers.indices.get(),
                font_texture.get(),
                tweak_values::verb_color,
                false,
                text_buffers.indices->Size());
        }
    }
}

math::Quad InteractionSystemDrawer::BoundingBox() const
{
    return math::InfQuad;
}
