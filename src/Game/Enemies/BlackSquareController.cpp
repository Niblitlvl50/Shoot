
#include "BlackSquareController.h"

#include "Player/PlayerInfo.h"
#include "Behaviour/HomingBehaviour.h"
#include "CollisionConfiguration.h"

#include "EventHandler/EventHandler.h"

#include "Events/ShockwaveEvent.h"

#include "Physics/IBody.h"
#include "Physics/PhysicsSystem.h"

#include "Rendering/Color.h"
#include "Rendering/Sprite/ISprite.h"
#include "Rendering/Sprite/Sprite.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/RenderSystem.h"

#include "Math/MathFunctions.h"

#include "SystemContext.h"
#include "TransformSystem/TransformSystem.h"

using namespace game;


BlackSquareController::BlackSquareController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler& event_handler)
    : m_entity_id(entity_id)
    , m_event_handler(event_handler)
    , m_trigger_distance(5.0f)
    , m_awake_state_timer(0)
{
    mono::TransformSystem* transform_system = system_context->GetSystem<mono::TransformSystem>();
    m_transform = &transform_system->GetTransform(entity_id);

    mono::SpriteSystem* sprite_system = system_context->GetSystem<mono::SpriteSystem>();
    m_sprite = sprite_system->GetSprite(entity_id);

    mono::PhysicsSystem* physics_system = system_context->GetSystem<mono::PhysicsSystem>();
    m_body = physics_system->GetBody(entity_id);
    m_body->AddCollisionHandler(this);

    m_homing_behaviour = std::make_unique<HomingBehaviour>(m_body, physics_system);

    using namespace std::placeholders;

    const std::unordered_map<States, MyStateMachine::State>& state_table = {
        { States::SLEEPING, { std::bind(&BlackSquareController::ToSleep, this), std::bind(&BlackSquareController::SleepState, this, _1) } },
        { States::AWAKE,    { std::bind(&BlackSquareController::ToAwake, this), std::bind(&BlackSquareController::AwakeState, this, _1) } },
        { States::HUNT,     { std::bind(&BlackSquareController::ToHunt,  this), std::bind(&BlackSquareController::HuntState, this, _1)  } }
    };

    m_states.SetStateTable(state_table);
    m_states.TransitionTo(States::SLEEPING);
}

BlackSquareController::~BlackSquareController()
{ }

void BlackSquareController::Update(const mono::UpdateContext& update_context)
{
    m_states.UpdateState(update_context.delta_ms);
}

mono::CollisionResolve BlackSquareController::OnCollideWith(
    mono::IBody* body, const math::Vector& collision_point, const math::Vector& collision_normal, uint32_t category)
{
    if(m_states.ActiveState() == States::SLEEPING)
        m_states.TransitionTo(States::AWAKE);

    if(category == shared::CollisionCategory::PLAYER)
    {
        const math::Vector& entity_position = math::GetPosition(*m_transform);
        //const float direction = math::AngleBetweenPoints(entity_position, body->GetPosition());

        //m_event_handler.DispatchEvent(game::DamageEvent(body, 45, direction));
        m_event_handler.DispatchEvent(game::ShockwaveEvent(entity_position, 100));
        //m_event_handler.DispatchEvent(game::RemoveEntityEvent(m_entity_id));
    }

    return mono::CollisionResolve::NORMAL;
}

void BlackSquareController::OnSeparateFrom(mono::IBody* body)
{

}

void BlackSquareController::ToSleep()
{
    m_sprite->SetShade(mono::Color::RGBA(0.0f, 0.0f, 0.0f, 1.0f));
    m_body->ResetForces();

    const math::Vector& entity_position = math::GetPosition(*m_transform);
    m_homing_behaviour->SetHomingPosition(entity_position);
}

void BlackSquareController::ToAwake()
{
    m_sprite->SetShade(mono::Color::RGBA(1.0f, 1.0f, 1.0f, 1.0f));
    m_awake_state_timer = 0;
}

void BlackSquareController::ToHunt()
{
    m_sprite->SetShade(mono::Color::RGBA(1.0f, 0.0f, 0.0f, 1.0f));
}

void BlackSquareController::SleepState(uint32_t delta)
{
    const math::Vector& entity_position = math::GetPosition(*m_transform);
    const game::PlayerInfo* player_info = GetClosestActivePlayer(entity_position);
    if(!player_info)
        return;

    m_homing_behaviour->SetHomingPosition(entity_position);

    const float distance = math::Length(player_info->position - entity_position);
    if(distance < m_trigger_distance)
        m_states.TransitionTo(States::AWAKE);
}

void BlackSquareController::AwakeState(uint32_t delta)
{
    m_awake_state_timer += delta;
    if(m_awake_state_timer > 300)
        m_states.TransitionTo(States::HUNT);
}

void BlackSquareController::HuntState(uint32_t delta)
{
    const math::Vector& entity_position = math::GetPosition(*m_transform);
    const game::PlayerInfo* player_info = GetClosestActivePlayer(entity_position);
    if(!player_info)
    {
        m_states.TransitionTo(States::SLEEPING);
        return;
    }

    const float angle = math::AngleBetweenPoints(player_info->position, entity_position) - math::PI_2();
    m_body->SetAngle(angle);

    m_homing_behaviour->SetHomingPosition(player_info->position);

    const float distance = math::Length(player_info->position - entity_position);
    if(distance > (m_trigger_distance * 2.0f))
        m_states.TransitionTo(States::SLEEPING);
}
