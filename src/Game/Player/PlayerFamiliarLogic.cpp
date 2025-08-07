
#include "PlayerFamiliarLogic.h"

#include "Player/PlayerInfo.h"

#include "EntitySystem/IEntityManager.h"
#include "Math/CriticalDampedSpring.h"
#include "Particle/ParticleSystem.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Sprite/Sprite.h"
#include "Rendering/Sprite/SpriteProperties.h"
#include "TransformSystem/TransformSystem.h"

#include "SystemContext.h"


namespace tweak_values
{
    constexpr float idle_threshold_s = 2.0f;
    constexpr float move_halflife = 0.3f;
}

using namespace game;

PlayerFamiliarLogic::PlayerFamiliarLogic(uint32_t entity_id, uint32_t owner_entity_id, mono::SystemContext* system_context)
    : m_entity_id(entity_id)
    , m_owner_entity_id(owner_entity_id)
    , m_last_show_state(false)
    , m_idle_timer(tweak_values::idle_threshold_s)
{
    m_entity_system = system_context->GetSystem<mono::IEntityManager>();
    m_sprite_system = system_context->GetSystem<mono::SpriteSystem>();
    m_transform_system = system_context->GetSystem<mono::TransformSystem>();
    m_particle_system = system_context->GetSystem<mono::ParticleSystem>();

    m_entity_system->SetEntityEnabled(m_entity_id, false);
}

void PlayerFamiliarLogic::Update(const mono::UpdateContext& update_context)
{
    const game::PlayerInfo* player_info = game::FindPlayerInfoFromEntityId(m_owner_entity_id);
    if(!player_info)
        return;

    const float velocity_length = math::Length(player_info->velocity);
    if(math::IsPrettyMuchEquals(velocity_length, 0.0f))
        m_idle_timer += update_context.delta_s;
    else
        m_idle_timer = 0.0f;

    const bool show_sprite = (m_idle_timer < tweak_values::idle_threshold_s) && (player_info->player_state == PlayerState::ALIVE);
    m_entity_system->SetEntityEnabled(m_entity_id, show_sprite);

    if(show_sprite != m_last_show_state)
    {
        const std::vector<mono::ParticleEmitterComponent*>& emitters = m_particle_system->GetAttachedEmitters(m_entity_id);
        if(!emitters.empty())
            m_particle_system->RestartEmitter(emitters.front());

        m_last_show_state = show_sprite;
    }

    if(show_sprite)
    {
        math::Vector current_position = m_transform_system->GetWorldPosition(m_entity_id);
        const math::Vector target_position =
            player_info->position - (math::RotateAroundZero(player_info->aim_direction, math::ToRadians(65.0f)) * 0.75f);

        math::critical_spring_damper(
            current_position,
            m_move_velocity,
            target_position,
            math::ZeroVec,
            tweak_values::move_halflife,
            update_context.delta_s);

        const bool face_left = (current_position.x > player_info->aim_target.x);
        mono::Sprite* sprite = m_sprite_system->GetSprite(m_entity_id);

        if(face_left)
            sprite->SetProperty(mono::SpriteProperty::FLIP_HORIZONTAL);
        else
            sprite->ClearProperty(mono::SpriteProperty::FLIP_HORIZONTAL);

        const math::Matrix new_transform = math::CreateMatrixWithPosition(current_position);
        m_transform_system->SetTransform(m_entity_id, new_transform);
    }
}
