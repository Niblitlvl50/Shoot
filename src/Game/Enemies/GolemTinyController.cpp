
#include "GolemTinyController.h"

#include "CollisionConfiguration.h"
#include "DamageSystem/DamageSystem.h"
#include "DamageSystem/Shockwave.h"
#include "Debug/IDebugDrawer.h"
#include "Entity/TargetSystem.h"
#include "Navigation/NavigationSystem.h"
#include "Weapons/WeaponTypes.h"

#include "Math/MathFunctions.h"
#include "Physics/IBody.h"
#include "Physics/PhysicsSystem.h"
#include "Physics/PhysicsSpace.h"

#include "Rendering/Color.h"
#include "Rendering/Sprite/ISprite.h"
#include "Rendering/Sprite/Sprite.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Sprite/SpriteProperties.h"
#include "SystemContext.h"

#include "Paths/IPath.h"
#include "Paths/PathFactory.h"
#include "EntitySystem/IEntityManager.h"
#include "TransformSystem/TransformSystem.h"


namespace tweak_values
{
    constexpr float engage_distance = 4.0f;
    //constexpr float disengage_distance = 5.0f;

    //constexpr float time_before_hunt_s = 0.3f;
    //constexpr float visibility_check_interval_s = 1.0f;
    constexpr float retarget_delay_s = 1.5f;

    //constexpr uint32_t collision_damage = 25;

    constexpr float stomp_distance = 1.0f;
    constexpr float roll_distance = 3.0f;
    constexpr float shockwave_radius = 2.0f;
    constexpr float shockwave_magnitude = 5.0f;

    //constexpr float degrees_per_second = 180.0f;
    constexpr float velocity_m_per_s = 0.4f;
}

using namespace game;

GolemTinyController::GolemTinyController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler* event_handler)
    : m_entity_id(entity_id)
{
    m_transform_system = system_context->GetSystem<mono::TransformSystem>();
    m_physics_system = system_context->GetSystem<mono::PhysicsSystem>();
    m_navigation_system = system_context->GetSystem<game::NavigationSystem>();
    m_entity_manager = system_context->GetSystem<mono::IEntityManager>();
    m_damage_system = system_context->GetSystem<game::DamageSystem>();
    m_target_system = system_context->GetSystem<game::TargetSystem>();

    mono::SpriteSystem* sprite_system = system_context->GetSystem<mono::SpriteSystem>();
    m_sprite = sprite_system->GetSprite(entity_id);

    mono::IBody* body = m_physics_system->GetBody(entity_id);
    body->AddCollisionHandler(this);
    
    m_tracking_movement.Init(body, m_navigation_system);
    m_tracking_movement.SetTrackingSpeed(tweak_values::velocity_m_per_s);

    m_path_behaviour.Init(body);

    using namespace std::placeholders;

    const MyStateMachine::StateTable& state_table = {
        MyStateMachine::MakeState(States::IDLE,         &GolemTinyController::ToIdle,       &GolemTinyController::IdleState, this),
        MyStateMachine::MakeState(States::WANDER,       &GolemTinyController::ToWander,     &GolemTinyController::TrackingState, this),
        MyStateMachine::MakeState(States::TRACKING,     &GolemTinyController::ToTracking,   &GolemTinyController::TrackingState, &GolemTinyController::ExitTracking, this),
        MyStateMachine::MakeState(States::STOMP_ATTACK, &GolemTinyController::ToStompAttack,&GolemTinyController::StompAttackState, &GolemTinyController::ExitAttack, this),
        MyStateMachine::MakeState(States::ROLL_ATTACK,  &GolemTinyController::ToRollAttack, &GolemTinyController::RollAttackState, &GolemTinyController::ExitAttack, this),
    };
    m_states.SetStateTableAndState(state_table, States::IDLE);
}

GolemTinyController::~GolemTinyController()
{ }

void GolemTinyController::Update(const mono::UpdateContext& update_context)
{
    if(m_update_spawn_position)
    {
        m_spawn_position = m_transform_system->GetWorldPosition(m_entity_id);
        m_update_spawn_position = false;
    }

    m_states.UpdateState(update_context);

    if(m_aquired_target && m_aquired_target->IsValid())
    {
        const math::Vector& entity_position = m_transform_system->GetWorldPosition(m_entity_id);
        
        const bool perform_roll_attack = m_aquired_target->IsWithinDistance(entity_position, tweak_values::roll_distance);
        if(perform_roll_attack)
            m_states.TransitionTo(States::ROLL_ATTACK);

        const bool perform_stomp_attack = m_aquired_target->IsWithinDistance(entity_position, tweak_values::stomp_distance);
        if(perform_stomp_attack)
            m_states.TransitionTo(States::STOMP_ATTACK);
    }

    const mono::IBody* body = m_physics_system->GetBody(m_entity_id);
    const math::Vector& velocity = body->GetVelocity();
    if(velocity.x < 0.0f)
        m_sprite->SetProperty(mono::SpriteProperty::FLIP_HORIZONTAL);
    else
        m_sprite->ClearProperty(mono::SpriteProperty::FLIP_HORIZONTAL);
}

void GolemTinyController::DrawDebugInfo(IDebugDrawer* debug_drawer) const
{
    const math::Vector world_position = m_transform_system->GetWorldPosition(m_entity_id);
    debug_drawer->DrawCircle(world_position, tweak_values::engage_distance, mono::Color::MAGENTA);
    debug_drawer->DrawCircle(world_position, tweak_values::stomp_distance, mono::Color::RED);
    debug_drawer->DrawCircle(world_position, tweak_values::roll_distance, mono::Color::ORANGE);

    const auto state_to_string = [](States state) {
        switch(state)
        {
        case States::IDLE:
            return "Idle";
        case States::WANDER:
            return "Wander";
        case States::TRACKING:
            return "Tracking";
        case States::STOMP_ATTACK:
            return "Stomp Attack";
        case States::ROLL_ATTACK:
            return "Roll Attack";
        }

        return "Unknown";
    };

    const char* active_state = state_to_string(m_states.ActiveState());
    const char* previous_state = state_to_string(m_states.PreviousState());

    char buffer[1024] = { };
    std::snprintf(buffer, std::size(buffer), "%s (%s)", active_state, previous_state);
    debug_drawer->DrawWorldText(buffer, world_position, mono::Color::OFF_WHITE);
}

const char* GolemTinyController::GetDebugCategory() const
{
    return "Tiny Golem Monster";
}

mono::CollisionResolve GolemTinyController::OnCollideWith(
    mono::IBody* body, const math::Vector& collision_point, const math::Vector& collision_normal, uint32_t category)
{
    /*
    if(m_states.ActiveState() == States::IDLE)
        m_visibility_check_timer_s = tweak_values::visibility_check_interval_s;

    if(category == CollisionCategory::PLAYER || category == CollisionCategory::PROPS)
    {
        const math::Vector& entity_position = m_transform_system->GetWorldPosition(m_entity_id);
        game::ShockwaveAt(m_physics_system, entity_position, tweak_values::shockwave_radius, tweak_values::shockwave_magnitude);
        
        const uint32_t other_entity_id = mono::PhysicsSystem::GetIdFromBody(body);
        m_damage_system->ApplyDamage(other_entity_id, m_entity_id, NO_WEAPON_IDENTIFIER, DamageDetails(tweak_values::collision_damage, false, false, false));
        m_damage_system->ApplyDamage(m_entity_id, m_entity_id, NO_WEAPON_IDENTIFIER, DamageDetails(1000, false, false, false));
    }
    */

    return mono::CollisionResolve::NORMAL;
}

void GolemTinyController::OnSeparateFrom(mono::IBody* body)
{ }

void GolemTinyController::ToIdle()
{
    m_retarget_timer_s = 0.0f;
    m_sprite->SetAnimation("idle");
}

void GolemTinyController::IdleState(const mono::UpdateContext& update_context)
{
    m_retarget_timer_s += update_context.delta_s;

    if(m_retarget_timer_s < tweak_values::retarget_delay_s)
        return;

    // Try to aquire a new target
    const math::Vector& entity_position = m_transform_system->GetWorldPosition(m_entity_id);
    m_aquired_target = m_target_system->AquireTarget(
        TargetFaction::Player, entity_position, tweak_values::engage_distance);

    const States new_state = m_aquired_target->IsValid() ? States::TRACKING : States::WANDER;
    m_states.TransitionTo(new_state);
}

void GolemTinyController::ToWander()
{
    const math::Vector& point_1m_away = math::RandomPointInCircle(1.0f);
    m_tracking_movement.UpdatePath(m_spawn_position + point_1m_away);

    m_sprite->SetAnimation("walk");
}

void GolemTinyController::ToTracking()
{
    const bool target_is_valid = m_aquired_target->IsValid();
    if(target_is_valid)
        m_tracking_movement.UpdatePath(m_aquired_target->Position());
    else
        m_states.TransitionTo(States::IDLE);

    m_sprite->SetAnimation("walk");
}

void GolemTinyController::ExitTracking()
{
    m_update_spawn_position = true;
}

void GolemTinyController::TrackingState(const mono::UpdateContext& update_context)
{
    const TrackingResult result = m_tracking_movement.Run(update_context);
    switch(result.state)
    {
    case TrackingState::NO_PATH:
        m_states.TransitionTo(States::IDLE);
        break;
 
    case TrackingState::TRACKING:
        break;

    case TrackingState::AT_TARGET:
        m_states.TransitionTo(States::IDLE);
        break;
    }
}

void GolemTinyController::ToStompAttack()
{
    const mono::SpriteAnimationCallback callback = [this](uint32_t sprite_id) {
        
        const math::Vector& entity_position = m_transform_system->GetWorldPosition(m_entity_id);
        game::ShockwaveAndDamageAt(
            m_physics_system,
            m_damage_system,
            entity_position,
            tweak_values::shockwave_radius,
            tweak_values::shockwave_magnitude,
            25,
            m_entity_id,
            CollisionCategory::PLAYER | CollisionCategory::PLAYER_BULLET | CollisionCategory::PACKAGE);

        m_states.TransitionTo(States::IDLE);
    };
    m_sprite->SetAnimation("attack", callback);
}

void GolemTinyController::StompAttackState(const mono::UpdateContext& update_context)
{ }

void GolemTinyController::ToRollAttack()
{
    if(!m_aquired_target || !m_aquired_target->IsValid())
    {
        m_states.TransitionTo(States::IDLE);
        return;
    }

    const math::Vector& entity_position = m_transform_system->GetWorldPosition(m_entity_id);
    const math::Vector& target_position = m_aquired_target->Position();

    const std::vector<math::Vector>& path_points = {
        entity_position, target_position
    };

    mono::IPathPtr path_ptr = mono::CreatePath(path_points);
    m_path_behaviour.SetPath(std::move(path_ptr));

    m_perform_roll_attack = false;

    const auto callback = [this](uint32_t sprite_id) {
        m_perform_roll_attack = true;
    };
    m_sprite->SetAnimation("prepare_roll", callback);
}

void GolemTinyController::RollAttackState(const mono::UpdateContext& update_context)
{
    if(!m_perform_roll_attack)
        return;

    const PathResult path_result = m_path_behaviour.Run(update_context.delta_s);
    if(path_result.distance_to_target < 0.1f || path_result.is_stuck)
        m_states.TransitionTo(States::IDLE);
}

void GolemTinyController::ExitAttack()
{
    m_update_spawn_position = true;
    m_aquired_target = nullptr;
}