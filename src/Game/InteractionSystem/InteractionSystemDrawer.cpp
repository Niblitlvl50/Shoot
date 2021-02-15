
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
#include "TransformSystem/TransformSystem.h"

using namespace game;

InteractionSystemDrawer::InteractionSystemDrawer(
    InteractionSystem* interaction_system,
    mono::SpriteSystem* sprite_system,
    const mono::TransformSystem* transform_system)
    : m_interaction_system(interaction_system)
    , m_sprite_system(sprite_system)
    , m_transform_system(transform_system)
{
    m_sprite = mono::GetSpriteFactory()->CreateSprite("res/sprites/emote_star.sprite");
    m_buffers = mono::BuildSpriteDrawBuffers(m_sprite->GetSpriteData());

    constexpr uint16_t indices[] = {
        0, 1, 2, 0, 2, 3
    };
    m_indices = mono::CreateElementBuffer(mono::BufferType::STATIC, 6, indices);
}

void InteractionSystemDrawer::Draw(mono::IRenderer& renderer) const
{
    std::vector<math::Matrix> active_interactions;

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

        const math::Quad& entity_world_bb = m_transform_system->GetWorldBoundingBox(interaction_trigger.trigger_id);
        const math::Vector& entity_position = math::TopCenter(entity_world_bb) + math::Vector(-0.1f, 0.2f);
        const math::Matrix transform = math::CreateMatrixWithPosition(entity_position);
        active_interactions.push_back(transform);
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

    for(const math::Matrix& transform : active_interactions)
    {
        const auto scope = mono::MakeTransformScope(transform, &renderer);
        const int offset = m_sprite->GetCurrentFrameIndex() * m_buffers.vertices_per_sprite;
        mono::ITexture* texture = m_sprite->GetTexture();

        renderer.DrawSprite(
            m_sprite.get(),
            m_buffers.vertices.get(),
            m_buffers.offsets.get(),
            m_buffers.uv.get(),
            m_buffers.uv_flipped.get(),
            m_buffers.heights.get(),
            m_indices.get(),
            texture,
            offset);
    }
}

math::Quad InteractionSystemDrawer::BoundingBox() const
{
    return math::InfQuad;
}
