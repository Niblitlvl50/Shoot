
#include "HookshotLogic.h"
#include "Physics/PhysicsSystem.h"
#include "Physics/PhysicsSpace.h"
#include "Physics/IConstraint.h"
#include "Math/MathFunctions.h"

#include "EntitySystem/IEntityManager.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "TransformSystem/TransformSystem.h"

#include "CollisionConfiguration.h"
#include "Entity/EntityLogicSystem.h"
#include "Entity/IEntityLogic.h"
#include "Entity/Component.h"
#include "Debug/IDebugDrawer.h"


#include "Rendering/RenderSystem.h"
#include "Rendering/RenderBuffer/IRenderBuffer.h"
#include "Rendering/Texture/ITextureFactory.h"
#include "Paths/PathFactory.h"

namespace
{
    const char* GetStateString(game::Hookshot::States state)
    {
        switch(state)
        {
        case game::Hookshot::States::IDLE:
            return "IDLE";
        case game::Hookshot::States::ANIMATING:
            return "ANIMATING";
        case game::Hookshot::States::ATTACHED:
            return "ATTACHED";
        case game::Hookshot::States::DETACHED:
            return "DETACHED";
        case game::Hookshot::States::MISSED:
            return "MISSED";
        }

        return "Unknown";
    }

    class HookshotLogic : public game::IEntityLogic
    {
    public:
        HookshotLogic(game::Hookshot* hookshot)
            : m_hookshot(hookshot)
        {
            m_texture = mono::RenderSystem::GetTextureFactory()->CreateTexture("res/textures/particles/rope_vertical1.png");
        }

        void Draw(mono::IRenderer& renderer) const override
        {
            const game::Hookshot::States state = m_hookshot->GetState();
            if(state != game::Hookshot::States::ATTACHED)
                return;

            const math::Vector attached_to_point = m_hookshot->GetAttachedPoint();
            const math::Vector hookshot_point = m_hookshot->GetHookshotPoint();

            std::vector<math::Vector> points = { hookshot_point, attached_to_point };

            mono::PathOptions options;
            options.closed = false;
            options.width = 0.0625f; // 2 pixels wide is the texture, thus 2/32 since the game runs as 32 pixels per meter.
            options.color = mono::Color::WHITE;
            options.uv_mode = mono::UVMode(mono::UVMode::DISTANCE | mono::UVMode::NORMALIZED_WIDTH);

            mono::PathDrawBuffer m_path_buffers = mono::BuildPathDrawBuffers(mono::PathType::REGULAR, points, options);

            renderer.DrawAnnotatedTrianges(
                m_path_buffers.vertices.get(),
                m_path_buffers.anotations.get(),
                m_path_buffers.indices.get(),
                m_texture.get(),
                mono::Color::WHITE,
                0,
                m_path_buffers.indices->Size());
        }

        void DrawDebugInfo(game::IDebugDrawer* debug_drawer) const override
        {
            const game::Hookshot::States state = m_hookshot->GetState();
            const char* state_string = GetStateString(state);
            debug_drawer->DrawWorldText(state_string, math::ZeroVec, mono::Color::MAGENTA);
        }

        const char* GetDebugCategory() const override
        {
            return "Hookshot";
        }


        game::Hookshot* m_hookshot;
        //mono::PathDrawBuffer m_path_buffers;
        mono::ITexturePtr m_texture;
    };
}

using namespace game;

Hookshot::Hookshot(
    uint32_t owner_entity_id,
    mono::IEntityManager* entity_system,
    mono::PhysicsSystem* physics_system,
    mono::SpriteSystem* sprite_system,
    mono::TransformSystem* transform_system,
    EntityLogicSystem* logic_system)
    : m_owner_entity_id(owner_entity_id)
    , m_entity_system(entity_system)
    , m_physics_system(physics_system)
    , m_sprite_system(sprite_system)
    , m_transform_system(transform_system)
    , m_hookshot_spring(nullptr)
{
    m_grappler_entity = entity_system->SpawnEntity("res/entities/hookshot_grappler.entity").id;
    entity_system->AddComponent(m_grappler_entity, BEHAVIOUR_COMPONENT);
    logic_system->AddLogic(m_grappler_entity, new HookshotLogic(this));

    m_sprite_system->SetSpriteEnabled(m_grappler_entity, false);

    const HookshotStateMachine::StateTable state_table = {
        HookshotStateMachine::MakeState(States::IDLE, &Hookshot::OnIdle, &Hookshot::Idle, this),
        HookshotStateMachine::MakeState(States::ANIMATING, &Hookshot::OnAnimating, &Hookshot::Animating, this),
        HookshotStateMachine::MakeState(States::ATTACHED, &Hookshot::OnAttached, &Hookshot::Attached, this),
        HookshotStateMachine::MakeState(States::DETACHED, &Hookshot::OnDetached, &Hookshot::Detached, this),
        HookshotStateMachine::MakeState(States::MISSED, &Hookshot::OnMissed, &Hookshot::Missed, this),
    };
    m_states.SetStateTableAndState(state_table, States::IDLE);
}

Hookshot::~Hookshot()
{
    m_entity_system->ReleaseEntity(m_grappler_entity);
}

void Hookshot::TriggerHookshot(const math::Vector& start, float direction)
{
    const States active_state = m_states.ActiveState();
    if(active_state != States::IDLE)
        return;

    m_attached_to_body = nullptr;

    constexpr float ray_length = 5.0f;
    const math::Vector unit_direction = math::VectorFromAngle(direction);
    const math::Vector end = start + (unit_direction * ray_length);
    const uint32_t categories = CollisionCategory::ENEMY | CollisionCategory::STATIC;

    const mono::QueryResult& result = m_physics_system->GetSpace()->QueryFirst(start, end, categories);
    if(result.body)
    {
        m_attached_to_body = result.body;
        m_attached_to_local_point = m_attached_to_body->WorldToLocal(result.point);
        m_rest_length = math::DistanceBetween(start, result.point);
        m_states.TransitionTo(States::ANIMATING);
    }
    else
    {
        m_states.TransitionTo(States::MISSED);
    }
}

void Hookshot::DetachHookshot()
{
    if(m_states.ActiveState() != States::ATTACHED)
        return;
        
    m_states.TransitionTo(States::DETACHED);
}

void Hookshot::ReleaseHookshot()
{
    if(!m_hookshot_spring)
        return;

    m_physics_system->ReleaseConstraint(m_hookshot_spring);
    m_hookshot_spring = nullptr;
    m_attached_to_body = nullptr;

    m_sprite_system->SetSpriteEnabled(m_grappler_entity, false);

    m_states.TransitionTo(States::IDLE);
}

void Hookshot::Update(const mono::UpdateContext& update_context)
{
    m_states.UpdateState(update_context);
}

Hookshot::States Hookshot::GetState() const
{
    return m_states.ActiveState();
}

mono::ConstraintBodyPair Hookshot::GetAttachedBodies() const
{
    if(!m_hookshot_spring)
        return { nullptr, nullptr };

    return m_hookshot_spring->GetBodies();
}

math::Vector Hookshot::GetAttachedPoint() const
{
    if(!m_attached_to_body)
        return math::ZeroVec;

    return m_attached_to_body->LocalToWorld(m_attached_to_local_point);
}

math::Vector Hookshot::GetHookshotPoint() const
{
    mono::IBody* owner_body = m_physics_system->GetBody(m_owner_entity_id);
    return owner_body->GetPosition();
}

void Hookshot::OnIdle()
{}
void Hookshot::Idle(const mono::UpdateContext& update_context)
{}

void Hookshot::OnAnimating()
{}
void Hookshot::Animating(const mono::UpdateContext& update_context)
{
    // Not sure about this state, i guess the travel time should be here.
    m_states.TransitionTo(States::ATTACHED);
}

void Hookshot::OnAttached()
{
    mono::IBody* owner_body = m_physics_system->GetBody(m_owner_entity_id);

//    m_hookshot_spring =
//        m_physics_system->CreateSpring(owner_body, m_attached_to_body, math::ZeroVec, m_attached_to_local_point, m_rest_length * 0.9f, 100.0, 0.0f);

    m_hookshot_spring =
        m_physics_system->CreateSlideJoint(owner_body, m_attached_to_body, math::ZeroVec, m_attached_to_local_point, 0.0, m_rest_length * 0.9f);

    m_sprite_system->SetSpriteEnabled(m_grappler_entity, true);
}
void Hookshot::Attached(const mono::UpdateContext& update_context)
{
    const math::Vector attached_point = GetAttachedPoint();
    const float angle = math::AngleBetweenPointsSimple(attached_point, GetHookshotPoint());

    m_transform_system->SetTransform(m_grappler_entity, math::CreateMatrixWithPositionRotation(attached_point, angle + math::PI_2()));

    //const mono::ConstraintBodyPair& body_pair = m_hookshot_spring->GetBodies();
    //const math::Vector first_position = body_pair.first->GetPosition();
    //const math::Vector second_position = body_pair.second->LocalToWorld(m_attached_to_local_point);

    //const float distance = math::DistanceBetween(first_position, second_position);
    //if(distance < 0.5f)
    //    m_states.TransitionTo(States::DETACHED);
}

void Hookshot::OnDetached()
{
    ReleaseHookshot();
}
void Hookshot::Detached(const mono::UpdateContext& update_context)
{
    m_states.TransitionTo(States::IDLE);
}

void Hookshot::OnMissed()
{}
void Hookshot::Missed(const mono::UpdateContext& update_context)
{
    // Missed animation or effect or something.
    m_states.TransitionTo(States::IDLE);
}
