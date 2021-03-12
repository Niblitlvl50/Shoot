
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
#include "TransformSystem/TransformSystem.h"

#include "FontIds.h"

using namespace game;

InteractionSystemDrawer::InteractionSystemDrawer(
    InteractionSystem* interaction_system,
    mono::SpriteSystem* sprite_system,
    const mono::TransformSystem* transform_system)
    : m_interaction_system(interaction_system)
    , m_sprite_system(sprite_system)
    , m_transform_system(transform_system)
{
    m_sprites.push_back(mono::GetSpriteFactory()->CreateSprite("res/sprites/emote_star.sprite"));
    m_sprites.push_back(mono::GetSpriteFactory()->CreateSprite("res/sprites/emote_swirl.sprite"));

    for(size_t index = 0; index < m_sprites.size(); ++index)
    {
        mono::ISpritePtr& sprite = m_sprites[index];
        m_sprite_buffers.push_back(mono::BuildSpriteDrawBuffers(sprite->GetSpriteData()));

        const shared::InteractionType type = shared::InteractionType(index);
        m_verb_buffers.push_back(
            mono::BuildTextDrawBuffers(shared::FontId::PIXELETTE_TINY, shared::InteractionTypeToVerb(type), mono::FontCentering::VERTICAL));
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
        const math::Vector& entity_position = math::RightCenter(entity_world_bb) + math::Vector(0.3f, 0.0f);
        const math::Matrix transform = math::CreateMatrixWithPosition(entity_position);
        active_interactions.push_back({ transform, (uint32_t)interaction_trigger.interaction_type });
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
        const auto scope = mono::MakeTransformScope(draw_data.transform, &renderer);

        const mono::ISpritePtr& sprite = m_sprites[draw_data.sprite_index];
        const mono::SpriteDrawBuffers& buffers = m_sprite_buffers[draw_data.sprite_index];

        mono::ITexture* texture = sprite->GetTexture();
        const int offset = sprite->GetCurrentFrameIndex() * buffers.vertices_per_sprite;

        renderer.DrawSprite(
            sprite.get(),
            buffers.vertices.get(),
            buffers.offsets.get(),
            buffers.uv.get(),
            buffers.uv_flipped.get(),
            buffers.heights.get(),
            m_indices.get(),
            texture,
            offset);

        const mono::TextDrawBuffers& text_buffers = m_verb_buffers[draw_data.sprite_index];
        const mono::ITexturePtr font_texture = mono::GetFontTexture(shared::FontId::PIXELETTE_TINY);

        math::Matrix text_transform = draw_data.transform;
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

math::Quad InteractionSystemDrawer::BoundingBox() const
{
    return math::InfQuad;
}
