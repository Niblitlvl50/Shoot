
#include "ShamanController.h"

#include "Math/MathFunctions.h"
#include "Physics/PhysicsSystem.h"
#include "Physics/IBody.h"
#include "Rendering/Sprite/ISprite.h"
#include "Rendering/Sprite/Sprite.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "TransformSystem/TransformSystem.h"
#include "Debug/IDebugDrawer.h"
#include "Util/Random.h"
#include "SystemContext.h"

#include "DamageSystem/DamageSystem.h"
#include "Entity/TargetSystem.h"

namespace tweak_values
{
    constexpr float activate_distance = 8.0f;
    constexpr float flee_distance = 3.5f;
    constexpr float heal_range = 4.5f;
    constexpr int   heal_amount = 15;
    constexpr float heal_cooldown_s = 4.5f;
    constexpr float cast_duration_s = 1.0f;
    constexpr float move_speed = 0.7f;
    constexpr float degrees_per_second = 200.0f;
}

using namespace game;

ShamanController::ShamanController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler* event_handler)
    : m_entity_id(entity_id)
    , m_heal_cooldown_s(0.0f)
    , m_cast_timer_s(0.0f)
{
    m_transform_system = system_context->GetSystem<mono::TransformSystem>();
    m_damage_system = system_context->GetSystem<game::DamageSystem>();
    m_target_system = system_context->GetSystem<game::TargetSystem>();

    mono::PhysicsSystem* physics_system = system_context->GetSystem<mono::PhysicsSystem>();
    mono::IBody* body = physics_system->GetBody(entity_id);

    m_homing_movement.SetBody(body);
    m_homing_movement.SetForwardVelocity(tweak_values::move_speed);
    m_homing_movement.SetAngularVelocity(tweak_values::degrees_per_second);

    mono::SpriteSystem* sprite_system = system_context->GetSystem<mono::SpriteSystem>();
    m_sprite = sprite_system->GetSprite(entity_id);

    m_idle_anim_id = m_sprite->GetAnimationIdFromName("idle");
    m_cast_anim_id = m_sprite->GetAnimationIdFromName("attack");
    if(m_cast_anim_id == -1)
        m_cast_anim_id = m_idle_anim_id;

    const ShamanStateMachine::StateTable state_table = {
        ShamanStateMachine::MakeState(States::IDLE,      &ShamanController::ToIdle,     &ShamanController::Idle,     this),
        ShamanStateMachine::MakeState(States::FLEE,      &ShamanController::ToFlee,     &ShamanController::Flee,     this),
        ShamanStateMachine::MakeState(States::HEAL_CAST, &ShamanController::ToHealCast, &ShamanController::HealCast, this),
    };
    m_states.SetStateTableAndState(state_table, States::IDLE);
}

void ShamanController::Update(const mono::UpdateContext& update_context)
{
    m_heal_cooldown_s = std::max(0.0f, m_heal_cooldown_s - update_context.delta_s);
    m_states.UpdateState(update_context);
}

void ShamanController::DrawDebugInfo(IDebugDrawer* debug_drawer) const
{
    const math::Vector world_position = m_transform_system->GetWorldPosition(m_entity_id);
    debug_drawer->DrawCircle(world_position, tweak_values::activate_distance, mono::Color::GREEN);
    debug_drawer->DrawCircle(world_position, tweak_values::flee_distance, mono::Color::RED);
    debug_drawer->DrawCircle(world_position, tweak_values::heal_range, mono::Color::YELLOW);

    const char* state_string = nullptr;
    switch(m_states.ActiveState())
    {
    case States::IDLE:      state_string = "Idle"; break;
    case States::FLEE:      state_string = "Flee"; break;
    case States::HEAL_CAST: state_string = "Heal Cast"; break;
    }
    debug_drawer->DrawWorldText(state_string, world_position, mono::Color::OFF_WHITE);
}

const char* ShamanController::GetDebugCategory() const
{
    return "Shaman";
}

void ShamanController::ToIdle()
{
    m_sprite->SetAnimation(m_idle_anim_id);
    m_sprite->SetShade(mono::Color::WHITE);
}

void ShamanController::Idle(const mono::UpdateContext& update_context)
{
    const math::Vector world_position = m_transform_system->GetWorldPosition(m_entity_id);

    m_player_target = m_target_system->AquireTarget(TargetFaction::Player, world_position, tweak_values::activate_distance);
    if(m_player_target->IsValid() && m_player_target->IsWithinDistance(world_position, tweak_values::flee_distance))
    {
        m_states.TransitionTo(States::FLEE);
        return;
    }

    if(m_heal_cooldown_s > 0.0f)
        return;

    // Look for a nearby injured ally to heal.
    for(const ITargetPtr& target : m_target_system->GetActiveTargets())
    {
        if(!target->IsValid())
            continue;

        const uint32_t target_id = target->TargetId();
        if(target_id == m_entity_id)
            continue;

        if(m_target_system->GetFaction(target_id) != TargetFaction::Enemies)
            continue;

        if(!target->IsWithinDistance(world_position, tweak_values::heal_range))
            continue;

        const DamageRecord* record = m_damage_system->GetDamageRecord(target_id);
        if(record && record->health < record->full_health)
        {
            m_states.TransitionTo(States::HEAL_CAST);
            return;
        }
    }
}

void ShamanController::ToFlee()
{
    if(!m_player_target->IsValid())
    {
        m_states.TransitionTo(States::IDLE);
        return;
    }

    const math::Vector world_position = m_transform_system->GetWorldPosition(m_entity_id);
    const math::Vector away = math::Normalized(world_position - m_player_target->Position());
    const float side = mono::Chance(50) ? 1.0f : -1.0f;
    const math::Vector flee_target = world_position + away * 3.0f + math::Perpendicular(away) * side;
    m_homing_movement.SetTargetPosition(flee_target);
    m_sprite->SetAnimation(m_idle_anim_id);
}

void ShamanController::Flee(const mono::UpdateContext& update_context)
{
    const HomingResult result = m_homing_movement.Run(update_context);

    const math::Vector world_position = m_transform_system->GetWorldPosition(m_entity_id);
    const bool player_safe = !m_player_target->IsValid() ||
        !m_player_target->IsWithinDistance(world_position, tweak_values::flee_distance * 1.5f);

    if(result.distance_to_target < 0.2f || player_safe)
        m_states.TransitionTo(States::IDLE);
}

void ShamanController::ToHealCast()
{
    m_cast_timer_s = 0.0f;
    m_sprite->SetShade(mono::Color::GREEN);
    m_sprite->SetAnimation(m_cast_anim_id);
}

void ShamanController::HealCast(const mono::UpdateContext& update_context)
{
    m_cast_timer_s += update_context.delta_s;
    if(m_cast_timer_s < tweak_values::cast_duration_s)
        return;

    // Heal all nearby allies.
    const math::Vector world_position = m_transform_system->GetWorldPosition(m_entity_id);
    for(const ITargetPtr& target : m_target_system->GetActiveTargets())
    {
        if(!target->IsValid())
            continue;

        const uint32_t target_id = target->TargetId();
        if(target_id == m_entity_id)
            continue;

        if(m_target_system->GetFaction(target_id) != TargetFaction::Enemies)
            continue;

        if(target->IsWithinDistance(world_position, tweak_values::heal_range))
            m_damage_system->GainHealth(target_id, tweak_values::heal_amount);
    }

    m_sprite->SetShade(mono::Color::WHITE);
    m_heal_cooldown_s = tweak_values::heal_cooldown_s;
    m_states.TransitionTo(States::IDLE);
}
