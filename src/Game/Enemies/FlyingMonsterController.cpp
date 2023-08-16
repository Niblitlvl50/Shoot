
#include "FlyingMonsterController.h"

#include "Player/PlayerInfo.h"
#include "Behaviour/TrackingBehaviour.h"
#include "Debug/IDebugDrawer.h"
#include "Weapons/IWeapon.h"
#include "Weapons/WeaponSystem.h"
#include "Weapons/CollisionCallbacks.h"
#include "AIUtils.h"
#include "Navigation/NavigationSystem.h"

#include "Rendering/Color.h"
#include "Rendering/Sprite/ISprite.h"
#include "Rendering/Sprite/Sprite.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Sprite/SpriteProperties.h"

#include "SystemContext.h"
#include "EntitySystem/IEntityManager.h"
#include "TransformSystem/TransformSystem.h"
#include "Physics/PhysicsSystem.h"
#include "Physics/PhysicsSpace.h"

#include "Math/MathFunctions.h"

#include "CollisionConfiguration.h"

#include <cmath>

namespace tweak_values
{
    constexpr float idle_time_s = 1.5f;
    constexpr float attack_start_delay_s = 0.5f;
    constexpr float attack_distance = 3.0f;
    constexpr float max_attack_distance = 3.5f;
    constexpr float track_to_player_distance = 5.0f;
    constexpr float loose_interest_distance = 7.0f;
    constexpr int bullets_to_emit = 3;
}

using namespace game;

FlyingMonsterController::FlyingMonsterController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler* event_handler)
    : m_entity_id(entity_id)
{
    m_transform_system = system_context->GetSystem<mono::TransformSystem>();
    m_physics_system = system_context->GetSystem<mono::PhysicsSystem>();
    m_sprite_system = system_context->GetSystem<mono::SpriteSystem>();
    m_entity_manager = system_context->GetSystem<mono::IEntityManager>();

    game::NavigationSystem* navigation_system = system_context->GetSystem<game::NavigationSystem>();

    game::WeaponSystem* weapon_system = system_context->GetSystem<game::WeaponSystem>();
    m_weapon = weapon_system->CreatePrimaryWeapon(entity_id, WeaponFaction::ENEMY);

    mono::IBody* entity_body = m_physics_system->GetBody(entity_id);
    m_tracking_behaviour = std::make_unique<TrackingBehaviour>(entity_body, m_physics_system, navigation_system);

    const FlyingMonsterStateMachine::StateTable& state_table = {
        FlyingMonsterStateMachine::MakeState(States::IDLE, &FlyingMonsterController::ToIdle, &FlyingMonsterController::Idle, this),
        FlyingMonsterStateMachine::MakeState(States::TRACKING, &FlyingMonsterController::ToTracking, &FlyingMonsterController::Tracking, this),
        FlyingMonsterStateMachine::MakeState(States::ATTACK_ANTICIPATION, &FlyingMonsterController::ToAttackAnticipation, &FlyingMonsterController::AttackAnticipation, this),
        FlyingMonsterStateMachine::MakeState(States::ATTACKING, &FlyingMonsterController::ToAttacking, &FlyingMonsterController::Attacking, this),
    };
    m_states.SetStateTableAndState(state_table, States::IDLE);
}

FlyingMonsterController::~FlyingMonsterController()
{ }

void FlyingMonsterController::Update(const mono::UpdateContext& update_context)
{
    m_states.UpdateState(update_context);
    m_weapon->UpdateWeaponState(update_context.timestamp);
}

void FlyingMonsterController::DrawDebugInfo(IDebugDrawer* debug_drawer) const
{
    const char* state_string = nullptr;

    switch(m_states.ActiveState())
    {
    case States::IDLE:
        state_string = "Idle";
        break;
    case States::TRACKING:
        state_string = "Tracking";
        break;
    case States::ATTACK_ANTICIPATION:
        state_string = "Attack Anticipation";
        break;
    case States::ATTACKING:
        state_string = "Attacking";
        break;
    }

    const math::Vector world_position = m_transform_system->GetWorldPosition(m_entity_id);
    debug_drawer->DrawWorldText(state_string, world_position, mono::Color::OFF_WHITE);
    debug_drawer->DrawCircle(world_position, tweak_values::track_to_player_distance, mono::Color::CYAN);
    debug_drawer->DrawCircle(world_position, tweak_values::attack_distance, mono::Color::RED);
    debug_drawer->DrawCircle(world_position, tweak_values::max_attack_distance, mono::Color::RED);
    debug_drawer->DrawCircle(world_position, tweak_values::loose_interest_distance, mono::Color::GREEN);

    const math::Vector& tracking_position = m_tracking_behaviour->GetTrackingPosition();
    debug_drawer->DrawLine({ world_position, tracking_position }, 1.0f, mono::Color::BLUE);
    debug_drawer->DrawPoint(tracking_position, 10.0f, mono::Color::RED);
}

const char* FlyingMonsterController::GetDebugCategory() const
{
    return "FlyingMonsterController";
}

void FlyingMonsterController::ToIdle()
{
    m_idle_timer_s = 0.0f;
    m_tracking_behaviour->UpdateEntityPosition();

    mono::ISprite* sprite = m_sprite_system->GetSprite(m_entity_id);
    sprite->SetShade(mono::Color::WHITE);
}

void FlyingMonsterController::Idle(const mono::UpdateContext& update_context)
{
    m_idle_timer_s += update_context.delta_s;
    if(m_idle_timer_s < tweak_values::idle_time_s)
        return;

    m_idle_timer_s = 0.0f;
    
    const math::Vector& position = m_transform_system->GetWorldPosition(m_entity_id);
    const game::PlayerInfo* player_info = GetClosestActivePlayer(position);
    if(!player_info)
        return;

    const float distance_to_player = math::DistanceBetween(position, player_info->position);
    if(distance_to_player > tweak_values::track_to_player_distance)
        return;

    const bool sees_player = SeesPlayer(m_physics_system, position, player_info->position);
    if(!sees_player)
        return;

    if(distance_to_player < tweak_values::max_attack_distance)
        m_states.TransitionTo(States::ATTACK_ANTICIPATION);
    else if(distance_to_player < tweak_values::track_to_player_distance)
        m_states.TransitionTo(States::TRACKING);
}

void FlyingMonsterController::ToTracking()
{ }

void FlyingMonsterController::Tracking(const mono::UpdateContext& update_context)
{
    const math::Vector& position = m_transform_system->GetWorldPosition(m_entity_id);
    const game::PlayerInfo* player_info = GetClosestActivePlayer(position);
    if(!player_info)
    {
        m_states.TransitionTo(States::IDLE);
        return;
    }

    const float distance_to_player = math::DistanceBetween(position, player_info->position);
    if(distance_to_player > tweak_values::loose_interest_distance)
    {
        m_states.TransitionTo(States::IDLE);
        return;
    }

    mono::ISprite* sprite = m_sprite_system->GetSprite(m_entity_id);

    const math::Vector delta = position - player_info->position;
    const bool left_of_player = (delta.x > 0.0f);
    if(left_of_player)
        sprite->SetProperty(mono::SpriteProperty::FLIP_HORIZONTAL);
    else
        sprite->ClearProperty(mono::SpriteProperty::FLIP_HORIZONTAL);

    if(distance_to_player < tweak_values::attack_distance)
    {
        const bool sees_player = SeesPlayer(m_physics_system, position, player_info->position);
        if(sees_player)
        {
            m_states.TransitionTo(States::ATTACK_ANTICIPATION);
            return;
        }
    }

    const TrackingResult result = m_tracking_behaviour->Run(update_context, player_info->position);
    if(result == TrackingResult::NO_PATH || result == TrackingResult::AT_TARGET)
        m_states.TransitionTo(States::IDLE);
}

void FlyingMonsterController::ToAttackAnticipation()
{
    const math::Vector& position = m_transform_system->GetWorldPosition(m_entity_id);
    const game::PlayerInfo* player_info = GetClosestActivePlayer(position);
    if(!player_info)
    {
        m_states.TransitionTo(States::IDLE);
        return;
    }

    const float distance_to_player = math::DistanceBetween(player_info->position, position);
    if(distance_to_player > tweak_values::max_attack_distance)
    {
        m_states.TransitionTo(States::IDLE);
        return;
    }

    m_attack_target = player_info;

    mono::ISprite* sprite = m_sprite_system->GetSprite(m_entity_id);
    sprite->SetShade(mono::Color::RGBA(1.0f, 0.25f, 0.25f));

    m_attack_anticipation_timer_s = 0.0f;
}

void FlyingMonsterController::AttackAnticipation(const mono::UpdateContext& update_context)
{
    m_attack_anticipation_timer_s += update_context.delta_s;
    if(m_attack_anticipation_timer_s > tweak_values::attack_start_delay_s)
        m_states.TransitionTo(States::ATTACKING);
}

void FlyingMonsterController::ToAttacking()
{
    m_bullets_fired = 0;
}

void FlyingMonsterController::Attacking(const mono::UpdateContext& update_context)
{
    if(m_bullets_fired < tweak_values::bullets_to_emit)
    {
        const math::Vector& position = m_transform_system->GetWorldPosition(m_entity_id);
        const game::WeaponState fire_state = m_weapon->Fire(position, m_attack_target->position, update_context.timestamp);
        if(fire_state == game::WeaponState::FIRE)
            m_bullets_fired++;
        else if(fire_state == game::WeaponState::OUT_OF_AMMO)
            m_weapon->Reload(update_context.timestamp);
    }
    else
    {
        m_states.TransitionTo(States::IDLE);
    }
}
