
#include "FlamingSkullBossController.h"

#include "AIUtils.h"
#include "Player/PlayerInfo.h"
#include "CollisionConfiguration.h"
#include "DamageSystem/DamageSystem.h"
#include "Shockwave.h"
#include "Debug/IDebugDrawer.h"

#include "Physics/IBody.h"
#include "Physics/PhysicsSystem.h"
#include "Physics/PhysicsSpace.h"

#include "Rendering/Color.h"
#include "Rendering/Sprite/ISprite.h"
#include "Rendering/Sprite/Sprite.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Sprite/SpriteProperties.h"
#include "SystemContext.h"

#include "EntitySystem/IEntityManager.h"
#include "TransformSystem/TransformSystem.h"


namespace tweak_values
{
    constexpr float trigger_distance = 5.0f;
    constexpr uint32_t time_before_hunt_ms = 300;
    constexpr uint32_t visibility_check_interval = 1000;
    constexpr uint32_t collision_damage = 25;
    constexpr float shockwave_radius = 2.0f;
    constexpr float shockwave_magnitude = 5.0f;

    constexpr float degrees_per_second = 180.0f;
    constexpr float velocity = 1.0f;
}

using namespace game;

FlamingSkullBossController::FlamingSkullBossController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler& event_handler)
    : m_entity_id(entity_id)
    , m_event_handler(event_handler)
    , m_awake_state_timer(0)
{
    m_transform_system = system_context->GetSystem<mono::TransformSystem>();
    m_transform = &m_transform_system->GetTransform(entity_id);

    mono::SpriteSystem* sprite_system = system_context->GetSystem<mono::SpriteSystem>();
    m_sprite = sprite_system->GetSprite(entity_id);

    m_physics_system = system_context->GetSystem<mono::PhysicsSystem>();
    mono::IBody* body = m_physics_system->GetBody(entity_id);
    body->AddCollisionHandler(this);

    m_homing_behaviour.SetBody(body);
    m_homing_behaviour.SetForwardVelocity(tweak_values::velocity);
    m_homing_behaviour.SetAngularVelocity(tweak_values::degrees_per_second);

    m_stagger_behaviour.SetChanceAndDuration(0.1f, 1.0f);

    m_entity_manager = system_context->GetSystem<mono::IEntityManager>();
    m_damage_system = system_context->GetSystem<game::DamageSystem>();

    using namespace std::placeholders;

    const MyStateMachine::StateTable& state_table = {
        MyStateMachine::MakeState(States::SLEEPING, &FlamingSkullBossController::ToSleep, &FlamingSkullBossController::SleepState, this),
        MyStateMachine::MakeState(States::AWAKE,    &FlamingSkullBossController::ToAwake, &FlamingSkullBossController::AwakeState, this),
        MyStateMachine::MakeState(States::HUNT,     &FlamingSkullBossController::ToHunt,  &FlamingSkullBossController::HuntState, this),
    };
    m_states.SetStateTableAndState(state_table, States::SLEEPING);
}

FlamingSkullBossController::~FlamingSkullBossController()
{ }

void FlamingSkullBossController::Update(const mono::UpdateContext& update_context)
{
    m_states.UpdateState(update_context);
    m_stagger_behaviour.Update(update_context);
}

void FlamingSkullBossController::DrawDebugInfo(IDebugDrawer* debug_drawer) const
{
    const math::Vector world_position = m_transform_system->GetWorldPosition(m_entity_id);
    debug_drawer->DrawCircle(world_position, tweak_values::trigger_distance, mono::Color::MAGENTA);
}

const char* FlamingSkullBossController::GetDebugCategory() const
{
    return "Flaming Skull Boss";
}

mono::CollisionResolve FlamingSkullBossController::OnCollideWith(
    mono::IBody* body, const math::Vector& collision_point, const math::Vector& collision_normal, uint32_t category)
{
    if(m_states.ActiveState() == States::SLEEPING)
        m_states.TransitionTo(States::AWAKE);

    if(category == CollisionCategory::PLAYER)
    {
        const math::Vector& entity_position = math::GetPosition(*m_transform);
        game::ShockwaveAt(m_physics_system, entity_position, tweak_values::shockwave_radius, tweak_values::shockwave_magnitude);

        const uint32_t other_entity_id = mono::PhysicsSystem::GetIdFromBody(body);
        m_damage_system->ApplyDamage(other_entity_id, tweak_values::collision_damage, m_entity_id);
        //m_damage_system->ApplyDamage(m_entity_id, 1000, m_entity_id);
    }
    else if(category == CollisionCategory::PLAYER_BULLET)
    {
        m_stagger_behaviour.TestForStaggering();
    }

    return mono::CollisionResolve::NORMAL;
}

void FlamingSkullBossController::OnSeparateFrom(mono::IBody* body)
{ }

void FlamingSkullBossController::ToSleep()
{
    m_visibility_check_timer = 0;
}

void FlamingSkullBossController::SleepState(const mono::UpdateContext& update_context)
{
    const math::Vector& entity_position = math::GetPosition(*m_transform);
    const game::PlayerInfo* player_info = GetClosestActivePlayer(entity_position);
    if(!player_info)
        return;

    const float distance = math::DistanceBetween(player_info->position, entity_position);
    if(distance < tweak_values::trigger_distance)
    {
        m_visibility_check_timer += update_context.delta_ms;

        if(m_visibility_check_timer > tweak_values::visibility_check_interval)
        {
            const bool sees_player = SeesPlayer(m_physics_system, entity_position, player_info);
            if(sees_player)
                m_states.TransitionTo(States::AWAKE);

            m_visibility_check_timer = 0;
        }
    }
}

void FlamingSkullBossController::ToAwake()
{
    m_awake_state_timer = 0;

    const math::Vector& entity_position = math::GetPosition(*m_transform);
    const game::PlayerInfo* player_info = GetClosestActivePlayer(entity_position);
    if(player_info)
    {
        const math::Vector delta = (entity_position - player_info->position);
        const float angle = math::AngleFromVector(delta);
        m_homing_behaviour.SetHeading(angle);
    }
}

void FlamingSkullBossController::AwakeState(const mono::UpdateContext& update_context)
{
    m_awake_state_timer += update_context.delta_ms;
    if(m_awake_state_timer > tweak_values::time_before_hunt_ms)
        m_states.TransitionTo(States::HUNT);
}

void FlamingSkullBossController::ToHunt()
{
    const math::Vector& entity_position = math::GetPosition(*m_transform);
    m_target_player_info = game::GetClosestActivePlayer(entity_position);
}

void FlamingSkullBossController::HuntState(const mono::UpdateContext& update_context)
{
    if(!m_target_player_info || m_target_player_info->player_state != PlayerState::ALIVE)
    {
        m_states.TransitionTo(States::SLEEPING);
        return;
    }

    const bool in_stagger = m_stagger_behaviour.IsStaggering();
    if(in_stagger)
        return;

    m_homing_behaviour.SetTargetPosition(m_target_player_info->position);
    const game::HomingResult result = m_homing_behaviour.Run(update_context);
    const math::Vector new_direction = math::VectorFromAngle(result.new_heading);

    if(new_direction.x < 0.0f)
        m_sprite->SetProperty(mono::SpriteProperty::FLIP_HORIZONTAL);
    else
        m_sprite->ClearProperty(mono::SpriteProperty::FLIP_HORIZONTAL);

    if(result.distance_to_target > tweak_values::trigger_distance)
        m_states.TransitionTo(States::SLEEPING);
}
