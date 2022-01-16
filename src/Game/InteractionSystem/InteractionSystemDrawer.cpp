
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
        "res/sprites/button_indication.sprite",
        "res/sprites/button_indication.sprite",
        "res/sprites/button_indication.sprite",
        "res/sprites/button_cross.sprite",
    };

    static_assert(std::size(g_interaction_type_sprite) == std::size(game::interaction_type_verb));
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

        m_verb_text_buffers.push_back(
            mono::BuildTextDrawBuffers(FontId::PIXELETTE_TINY, verb, mono::FontCentering::VERTICAL));
        m_verb_text_widths.push_back(mono::MeasureString(FontId::PIXELETTE_TINY, verb).x);

        VerbSpriteBuffer verb_sprite_buffer;
        verb_sprite_buffer.sprite = mono::GetSpriteFactory()->CreateSprite(g_interaction_type_sprite[index]);
        verb_sprite_buffer.sprite_buffer = mono::BuildSpriteDrawBuffers(verb_sprite_buffer.sprite->GetSpriteData());

        m_verb_sprites_buffers.push_back(std::move(verb_sprite_buffer));
    }

    for(const char* verb : interaction_type_verb)
    {
        m_verb_text_buffers.push_back(
            mono::BuildTextDrawBuffers(FontId::PIXELETTE_TINY, verb, mono::FontCentering::VERTICAL));
        m_verb_text_widths.push_back(mono::MeasureString(FontId::PIXELETTE_TINY, verb).x);
    }

    constexpr uint16_t indices[] = {
        0, 1, 2, 0, 2, 3
    };
    m_indices = mono::CreateElementBuffer(mono::BufferType::STATIC, 6, indices);
}

void InteractionSystemDrawer::Draw(mono::IRenderer& renderer) const
{
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
        const math::Vector& entity_position = math::TopCenter(entity_world_bb) + math::Vector(0.0f, 0.2f);

        const math::Matrix transform = math::CreateMatrixWithPosition(entity_position);

        const char* entity_name = interaction_trigger.draw_name ? m_entity_system->GetEntityName(interaction_trigger.interaction_id) : nullptr;
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

            const math::Vector text_size = mono::MeasureString(FontId::PIXELETTE_TINY, draw_data.name);
            const math::Vector half_text_size = (text_size / 2.0f) + math::Vector(padding, padding);

            const math::Quad background_quad(-half_text_size, half_text_size);
            renderer.DrawFilledQuad(background_quad, mono::Color::RGBA(0.2f, 0.2f, 0.2f, 0.7f));

            renderer.RenderText(FontId::PIXELETTE_TINY, draw_data.name, mono::Color::OFF_WHITE, mono::FontCentering::HORIZONTAL_VERTICAL);
        }

        // Verb
        {
            const float verb_width = m_verb_text_widths[draw_data.sprite_index];
            const float width_padding = 0.1f;

            const math::Matrix projection = math::Ortho(0.0f, 12.0f, 0.0f, 8.0f, 0.0f, 1.0f);
            const math::Matrix transform = math::CreateMatrixWithPosition(math::Vector(10.0f, 1.5f));

            const auto projection_scope = mono::MakeProjectionScope(projection, &renderer);
            const auto transform_scope = mono::MakeTransformScope(transform, &renderer);
            const auto view_scope = mono::MakeViewTransformScope(math::Matrix(), &renderer);

            const VerbSpriteBuffer& sprite_and_buffer = m_verb_sprites_buffers[draw_data.sprite_index];
            const mono::ISprite* button_sprite = sprite_and_buffer.sprite.get();
            const mono::SpriteDrawBuffers& button_buffers = sprite_and_buffer.sprite_buffer;

            const mono::SpriteFrame current_frame = button_sprite->GetCurrentFrame();
            const float half_sprite_width = current_frame.size.x / 2.0f;

            const math::Quad background_quad(
                -(half_sprite_width + width_padding), -0.25f,
                (current_frame.size.x + verb_width + width_padding), 0.25f);
            renderer.DrawFilledQuad(background_quad, mono::Color::RGBA(0.2f, 0.2f, 0.2f, 0.7f));

            mono::ITexture* texture = button_sprite->GetTexture();
            const int offset = sprite_and_buffer.sprite->GetCurrentFrameIndex() * button_buffers.vertices_per_sprite;

            renderer.DrawSprite(
                button_sprite,
                button_buffers.vertices.get(),
                button_buffers.offsets.get(),
                button_buffers.uv.get(),
                button_buffers.uv_flipped.get(),
                button_buffers.heights.get(),
                m_indices.get(),
                texture,
                offset);

            const mono::TextDrawBuffers& text_buffers = m_verb_text_buffers[draw_data.sprite_index];
            const mono::ITexturePtr font_texture = mono::GetFontTexture(FontId::PIXELETTE_TINY);

            math::Matrix text_transform = transform;
            math::Translate(text_transform, math::Vector(0.35f, 0.0f));
            const auto text_scope = mono::MakeTransformScope(text_transform, &renderer);

            renderer.RenderText(
                text_buffers.vertices.get(),
                text_buffers.uv.get(),
                text_buffers.indices.get(),
                font_texture.get(),
                mono::Color::WHITE);
        }
    }
}

math::Quad InteractionSystemDrawer::BoundingBox() const
{
    return math::InfQuad;
}
