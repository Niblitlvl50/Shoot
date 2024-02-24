
#include "HookshotLogic.h"
#include "Physics/PhysicsSystem.h"
#include "Physics/PhysicsSpace.h"
#include "Physics/IConstraint.h"
#include "Math/MathFunctions.h"

#include "EntitySystem/IEntityManager.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "TransformSystem/TransformSystem.h"

#include "CollisionConfiguration.h"

using namespace game;

HookshotLogic::HookshotLogic(
    uint32_t owner_entity_id,
    mono::IEntityManager* entity_system,
    mono::PhysicsSystem* physics_system,
    mono::SpriteSystem* sprite_system,
    mono::TransformSystem* transform_system)
    : m_owner_entity_id(owner_entity_id)
    , m_entity_system(entity_system)
    , m_physics_system(physics_system)
    , m_sprite_system(sprite_system)
    , m_transform_system(transform_system)
    , m_hookshot_spring(nullptr)
{
    m_grappler_entity = entity_system->SpawnEntity("res/entities/hookshot_grappler.entity").id;
    m_sprite_system->SetSpriteEnabled(m_grappler_entity, false);

    const HookshotStateMachine::StateTable state_table = {
        HookshotStateMachine::MakeState(States::IDLE, &HookshotLogic::OnIdle, &HookshotLogic::Idle, this),
        HookshotStateMachine::MakeState(States::ANIMATING, &HookshotLogic::OnAnimating, &HookshotLogic::Animating, this),
        HookshotStateMachine::MakeState(States::ATTACHED, &HookshotLogic::OnAttached, &HookshotLogic::Attached, this),
        HookshotStateMachine::MakeState(States::DETACHED, &HookshotLogic::OnDetached, &HookshotLogic::Detached, this),
        HookshotStateMachine::MakeState(States::MISSED, &HookshotLogic::OnMissed, &HookshotLogic::Missed, this),
    };
    m_states.SetStateTableAndState(state_table, States::IDLE);
}

HookshotLogic::~HookshotLogic()
{
    m_entity_system->ReleaseEntity(m_grappler_entity);
}

void HookshotLogic::TriggerHookshot(const math::Vector& start, float direction)
{
    const States active_state = m_states.ActiveState();
    if(active_state != States::IDLE)
        return;

    constexpr float ray_length = 5.0f;
    const math::Vector unit_direction = math::VectorFromAngle(direction);
    const math::Vector end = start + (unit_direction * ray_length);
    const uint32_t categories = CollisionCategory::ENEMY | CollisionCategory::STATIC;

    const mono::QueryResult& result = m_physics_system->GetSpace()->QueryFirst(start, end, categories);
    if(result.body)
    {
        m_attached_to_body = result.body;
        m_attached_to_local_point = m_attached_to_body->WorldToLocal(result.point);
        m_states.TransitionTo(States::ANIMATING);
    }
    else
    {
        m_states.TransitionTo(States::MISSED);
    }
}

void HookshotLogic::ReleaseHookshot()
{
    if(!m_hookshot_spring)
        return;

    mono::IBody* owner_body = m_physics_system->GetBody(m_owner_entity_id);
    owner_body->RemoveCollisionHandler(this);

    m_physics_system->ReleaseConstraint(m_hookshot_spring);
    m_hookshot_spring = nullptr;

    m_sprite_system->SetSpriteEnabled(m_grappler_entity, false);

    m_states.TransitionTo(States::IDLE);
}

void HookshotLogic::Update(const mono::UpdateContext& update_context)
{
    m_states.UpdateState(update_context);
}

void HookshotLogic::OnIdle()
{}
void HookshotLogic::Idle(const mono::UpdateContext& update_context)
{}

void HookshotLogic::OnAnimating()
{}
void HookshotLogic::Animating(const mono::UpdateContext& update_context)
{
    // Not sure about this state, i guess the travel time should be here.
    m_states.TransitionTo(States::ATTACHED);
}

void HookshotLogic::OnAttached()
{
    mono::IBody* owner_body = m_physics_system->GetBody(m_owner_entity_id);
    owner_body->AddCollisionHandler(this);

    m_hookshot_spring =
        m_physics_system->CreateSpring(owner_body, m_attached_to_body, math::ZeroVec, m_attached_to_local_point, 0.0f, 400.0, 0.0f);

    const uint32_t attached_to_entity_id = m_attached_to_body->GetId();
    
    m_transform_system->ChildTransform(m_grappler_entity, attached_to_entity_id);
    m_transform_system->SetTransform(m_grappler_entity, math::CreateMatrixWithPosition(m_attached_to_local_point));

    m_sprite_system->SetSpriteEnabled(m_grappler_entity, true);
}
void HookshotLogic::Attached(const mono::UpdateContext& update_context)
{
    const mono::ConstraintBodyPair& body_pair = m_hookshot_spring->GetBodies();
    const math::Vector first_position = body_pair.first->GetPosition();
    const math::Vector second_position = body_pair.second->LocalToWorld(m_attached_to_local_point);

    const float distance = math::DistanceBetween(first_position, second_position);
    if(distance < 0.5f)
        m_states.TransitionTo(States::DETACHED);
}

void HookshotLogic::OnDetached()
{
    ReleaseHookshot();
}
void HookshotLogic::Detached(const mono::UpdateContext& update_context)
{
    m_states.TransitionTo(States::IDLE);
}

void HookshotLogic::OnMissed()
{}
void HookshotLogic::Missed(const mono::UpdateContext& update_context)
{
    // Missed animation or effect or something.
    m_states.TransitionTo(States::IDLE);
}

mono::CollisionResolve HookshotLogic::OnCollideWith(
    mono::IBody* body, const math::Vector& collision_point, const math::Vector& collision_normal, uint32_t categories)
{
    ReleaseHookshot();
    return mono::CollisionResolve::NORMAL;
}

void HookshotLogic::OnSeparateFrom(mono::IBody* body)
{ }
