
#include "SummonerController.h"

#include "Math/MathFunctions.h"
#include "Physics/PhysicsSystem.h"
#include "Physics/IBody.h"
#include "Rendering/Sprite/ISprite.h"
#include "Rendering/Sprite/Sprite.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Lights/LightSystem.h"
#include "TransformSystem/TransformSystem.h"
#include "EntitySystem/EntitySystem.h"
#include "EntitySystem/Entity.h"
#include "Debug/IDebugDrawer.h"
#include "Util/Random.h"
#include "SystemContext.h"

#include "Entity/TargetSystem.h"
#include "Navigation/NavigationSystem.h"
#include "DamageSystem/DamageSystem.h"
#include "SpawnSystem/SpawnSystem.h"
#include "Effects/HealEffect.h"
#include "Particle/ParticleSystem.h"
#include <algorithm>

namespace tweak_values
{
    constexpr float activate_distance = 9.0f;
    constexpr float preferred_distance = 5.5f;
    constexpr float danger_distance = 3.0f;
    constexpr float prepare_duration_s = 1.5f;
    constexpr float summon_cooldown_s = 7.0f;
    constexpr float summon_radius = 1.5f;
    constexpr float move_speed = 0.6f;
    constexpr float retreat_speed = 1.2f;
    constexpr float degrees_per_second = 180.0f;
    constexpr float heal_radius = 4.0f;
    constexpr float heal_per_second = 5.0f;
    constexpr float heal_tick_interval_s = 0.5f;
    constexpr float heal_duration_s = 3.0f;
    constexpr float heal_cooldown_s = 8.0f;
    constexpr int max_minions = 3;
}

using namespace game;

SummonerController::SummonerController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler* event_handler)
    : m_entity_id(entity_id)
    , m_max_minions(tweak_values::max_minions)
    , m_spawn_point_component(nullptr)
    , m_prepare_timer_s(0.0f)
    , m_cooldown_timer_s(0.0f)
    , m_heal_duration_timer_s(0.0f)
    , m_heal_cooldown_timer_s(0.0f)
    , m_heal_tick_timer_s(0.0f)
{
    m_transform_system = system_context->GetSystem<mono::TransformSystem>();
    m_navigation_system = system_context->GetSystem<game::NavigationSystem>();
    m_target_system = system_context->GetSystem<game::TargetSystem>();
    m_damage_system = system_context->GetSystem<game::DamageSystem>();
    m_spawn_system = system_context->GetSystem<game::SpawnSystem>();
    m_entity_manager = system_context->GetSystem<mono::EntitySystem>();

    mono::ParticleSystem* particle_system = system_context->GetSystem<mono::ParticleSystem>();
    m_heal_effect = std::make_unique<HealEffect>(particle_system, m_entity_manager);

    mono::PhysicsSystem* physics_system = system_context->GetSystem<mono::PhysicsSystem>();
    mono::IBody* body = physics_system->GetBody(entity_id);

    m_homing_movement.SetBody(body);
    m_homing_movement.SetForwardVelocity(tweak_values::move_speed);
    m_homing_movement.SetAngularVelocity(tweak_values::degrees_per_second);

    m_tracking_movement.Init(body, m_navigation_system);
    m_tracking_movement.SetTrackingSpeed(tweak_values::move_speed);

    mono::SpriteSystem* sprite_system = system_context->GetSystem<mono::SpriteSystem>();
    m_sprite = sprite_system->GetSprite(entity_id);

    m_idle_anim_id = m_sprite->GetAnimationIdFromName("idle");
    m_walk_anim_id = m_sprite->GetAnimationIdFromName("walk");
    m_cast_anim_id = m_sprite->GetAnimationIdFromName("attack");

    m_spawn_point_component = m_spawn_system->GetSpawnPoint(m_entity_id);

    const SummonerStateMachine::StateTable state_table = {
        SummonerStateMachine::MakeState(States::IDLE,    &SummonerController::ToIdle,    &SummonerController::Idle,    this),
        SummonerStateMachine::MakeState(States::PREPARE, &SummonerController::ToPrepare, &SummonerController::Prepare, this),
        SummonerStateMachine::MakeState(States::SUMMON,  &SummonerController::ToSummon,  &SummonerController::Summon,  this),
        SummonerStateMachine::MakeState(States::COOLDOWN,&SummonerController::ToCooldown,&SummonerController::Cooldown,this),
        SummonerStateMachine::MakeState(States::HEAL,    &SummonerController::ToHeal,    &SummonerController::Heal,    this),
    };
    m_states.SetStateTableAndState(state_table, States::IDLE);
}

SummonerController::~SummonerController() = default;

void SummonerController::Update(const mono::UpdateContext& update_context)
{
    m_states.UpdateState(update_context);
    m_heal_cooldown_timer_s -= update_context.delta_s;
}

void SummonerController::DrawDebugInfo(IDebugDrawer* debug_drawer) const
{
    const math::Vector world_position = m_transform_system->GetWorldPosition(m_entity_id);
    debug_drawer->DrawCircle(world_position, tweak_values::activate_distance, mono::Color::GREEN);
    debug_drawer->DrawCircle(world_position, tweak_values::preferred_distance, mono::Color::CYAN);
    debug_drawer->DrawCircle(world_position, tweak_values::summon_radius, mono::Color::MAGENTA);

    const char* state_string = "Unknown";
    switch(m_states.ActiveState())
    {
    case States::IDLE:     state_string = "Idle"; break;
    case States::PREPARE:  state_string = "Prepare"; break;
    case States::SUMMON:   state_string = "Summon"; break;
    case States::COOLDOWN: state_string = "Cooldown"; break;
    case States::HEAL:     state_string = "Heal"; break;
    }
    debug_drawer->DrawWorldText(state_string, world_position, mono::Color::OFF_WHITE);
}

const char* SummonerController::GetDebugCategory() const
{
    return "Summoner";
}

int SummonerController::CountActiveMinions() const
{
    if(!m_spawn_point_component)
        return 0;
    return static_cast<int>(m_spawn_point_component->active_spawns.size());
}

void SummonerController::ToIdle()
{
    m_sprite->SetAnimation(m_idle_anim_id);
    m_sprite->SetShade(mono::Color::WHITE);
}

void SummonerController::Idle(const mono::UpdateContext& update_context)
{
    const math::Vector world_position = m_transform_system->GetWorldPosition(m_entity_id);

    m_player_target = m_target_system->AquireTarget(TargetFaction::Player, world_position, tweak_values::activate_distance);
    if(!m_player_target->IsValid())
        return;

    const float distance = math::DistanceBetween(world_position, m_player_target->Position());

    // Retreat if player is too close.
    if(distance < tweak_values::danger_distance)
    {
        const math::Vector away = math::Normalized(world_position - m_player_target->Position());
        m_homing_movement.SetForwardVelocity(tweak_values::retreat_speed);
        m_homing_movement.SetTargetPosition(world_position + away * tweak_values::preferred_distance);
        m_homing_movement.Run(update_context);
        m_sprite->SetAnimation(m_walk_anim_id);
        return;
    }

    // Move toward player until within preferred distance.
    if(distance > tweak_values::preferred_distance)
    {
        m_sprite->SetAnimation(m_walk_anim_id);
        m_tracking_movement.Run(update_context, m_player_target->Position());
        return;
    }

    m_sprite->SetAnimation(m_idle_anim_id);

    // Heal injured minions before summoning new ones.
    if(m_heal_cooldown_timer_s <= 0.0f && HasInjuredMinionInRange(world_position))
    {
        m_states.TransitionTo(States::HEAL);
        return;
    }

    // Summon if we have room and the cooldown is done.
    if(m_cooldown_timer_s <= 0.0f && CountActiveMinions() < m_max_minions)
        m_states.TransitionTo(States::PREPARE);
}

void SummonerController::ToPrepare()
{
    m_prepare_timer_s = 0.0f;
    m_sprite->SetShade(mono::Color::MAGENTA);
    m_sprite->SetAnimation(m_cast_anim_id);
}

void SummonerController::Prepare(const mono::UpdateContext& update_context)
{
    m_prepare_timer_s += update_context.delta_s;
    if(m_prepare_timer_s >= tweak_values::prepare_duration_s)
        m_states.TransitionTo(States::SUMMON);
}

void SummonerController::ToSummon()
{
    m_spawn_system->SpawnFromSpawnPoint(m_entity_id);
    m_states.TransitionTo(States::COOLDOWN);
}

void SummonerController::Summon(const mono::UpdateContext& update_context)
{ }

void SummonerController::ToCooldown()
{
    m_cooldown_timer_s = tweak_values::summon_cooldown_s;
    m_sprite->SetShade(mono::Color::WHITE);
    m_sprite->SetAnimation(m_idle_anim_id);
}

void SummonerController::Cooldown(const mono::UpdateContext& update_context)
{
    m_cooldown_timer_s -= update_context.delta_s;
    if(m_cooldown_timer_s <= 0.0f)
        m_states.TransitionTo(States::IDLE);
}

bool SummonerController::HasInjuredMinionInRange(const math::Vector& world_position) const
{
    if(!m_spawn_point_component)
        return false;

    for(const SpawnSystem::SpawnIdAndCallback& entry : m_spawn_point_component->active_spawns)
    {
        const DamageRecord* record = m_damage_system->GetDamageRecord(entry.spawned_entity_id);
        if(!record || record->health >= record->full_health)
            continue;

        const math::Vector minion_pos = m_transform_system->GetWorldPosition(entry.spawned_entity_id);
        if(math::DistanceBetween(world_position, minion_pos) <= tweak_values::heal_radius)
            return true;
    }
    return false;
}

void SummonerController::ToHeal()
{
    m_heal_duration_timer_s = tweak_values::heal_duration_s;
    m_heal_tick_timer_s = 0.0f;
    m_sprite->SetShade(mono::Color::GREEN);
    m_sprite->SetAnimation(m_cast_anim_id);
}

void SummonerController::Heal(const mono::UpdateContext& update_context)
{
    m_heal_duration_timer_s -= update_context.delta_s;
    if(m_heal_duration_timer_s <= 0.0f)
    {
        m_heal_cooldown_timer_s = tweak_values::heal_cooldown_s;
        m_sprite->SetShade(mono::Color::WHITE);
        m_states.TransitionTo(States::IDLE);
        return;
    }

    m_heal_tick_timer_s -= update_context.delta_s;
    if(m_heal_tick_timer_s > 0.0f)
        return;

    m_heal_tick_timer_s = tweak_values::heal_tick_interval_s;

    const int heal_amount = static_cast<int>(tweak_values::heal_per_second * tweak_values::heal_tick_interval_s);
    const math::Vector world_position = m_transform_system->GetWorldPosition(m_entity_id);

    if(m_spawn_point_component)
    {
        for(const SpawnSystem::SpawnIdAndCallback& entry : m_spawn_point_component->active_spawns)
        {
            const math::Vector minion_pos = m_transform_system->GetWorldPosition(entry.spawned_entity_id);
            if(math::DistanceBetween(world_position, minion_pos) <= tweak_values::heal_radius)
            {
                m_damage_system->GainHealth(entry.spawned_entity_id, heal_amount);
                m_heal_effect->EmitAt(minion_pos);
            }
        }
    }
}
