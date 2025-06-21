
#include "GamePhysicsSystem.h"
#include "CollisionConfiguration.h"

#include "Math/MathFunctions.h"
#include "Physics/PhysicsSystem.h"
#include "Physics/IShape.h"
#include "TransformSystem/TransformSystem.h"
#include "Util/Algorithm.h"

namespace
{
    class ImpulseShapeCollisionHandler : public mono::ICollisionHandler
    {
    public:

        ImpulseShapeCollisionHandler(uint32_t entity_id, game::GamePhysicsSystem* game_physics_system)
            : m_entity_id(entity_id)
            , m_game_physics_system(game_physics_system)
        { }

        mono::CollisionResolve OnCollideWith(
            mono::IBody* body, const math::Vector& collision_point, const math::Vector& collision_normal, uint32_t categories) override
        {
            const uint32_t id = body->GetId();

            const auto it = std::find(m_active_bodies.begin(), m_active_bodies.end(), id);
            if(it == m_active_bodies.end())
            {
                m_active_bodies.push_back(id);
                m_game_physics_system->ApplyImpulseFromIdTo(m_entity_id, body);
            }

            return mono::CollisionResolve::NORMAL;
        }

        void OnSeparateFrom(mono::IBody* body) override
        {
            const uint32_t id = body->GetId();
            mono::remove(m_active_bodies, id);
        }

        const uint32_t m_entity_id;
        game::GamePhysicsSystem* m_game_physics_system;

        std::vector<uint32_t> m_active_bodies;
    };
}

using namespace game;

GamePhysicsSystem::GamePhysicsSystem(mono::TransformSystem* transform_system, mono::PhysicsSystem* physics_system)
    : m_transform_system(transform_system)
    , m_physics_system(physics_system)
{ }

const char* GamePhysicsSystem::Name() const
{
    return "gamephysicssystem";
}

void GamePhysicsSystem::Update(const mono::UpdateContext& update_context)
{

}

ImpulseComponent* GamePhysicsSystem::AllocateImpulse(uint32_t entity_id)
{
    ImpulseComponent component;
    component.impulse_strength = 1.0f;
    component.entity_id = entity_id;

    m_impulse_components.push_back(std::move(component));
    return &m_impulse_components.back();
}

void GamePhysicsSystem::ReleaseImpulse(uint32_t entity_id)
{
    ImpulseComponent* component = FindImpulseComponent(entity_id);

    if(component->collision_handler)
    {
        mono::IBody* body = m_physics_system->GetBody(entity_id);
        if(body)
            body->RemoveCollisionHandler(component->collision_handler.get());

        component->collision_handler = nullptr;
    }

    const auto remove_by_id = [entity_id](const ImpulseComponent& component) {
        return component.entity_id == entity_id;
    };
    mono::remove_if(m_impulse_components, remove_by_id);
}

void GamePhysicsSystem::UpdateImpulse(uint32_t entity_id, float impulse_strength)
{
    ImpulseComponent* component = FindImpulseComponent(entity_id);
    component->impulse_strength = impulse_strength;

    mono::IBody* body = m_physics_system->GetBody(entity_id);
    if(body)
    {
        if(component->collision_handler)
            body->RemoveCollisionHandler(component->collision_handler.get());

        component->collision_handler = std::make_unique<ImpulseShapeCollisionHandler>(entity_id, this);
        body->AddCollisionHandler(component->collision_handler.get());
    }
}

ImpulseComponent* GamePhysicsSystem::FindImpulseComponent(uint32_t entity_id)
{
    const auto find_by_id = [entity_id](const ImpulseComponent& component) {
        return component.entity_id == entity_id;
    };

    return mono::find_if(m_impulse_components, find_by_id);
}

void GamePhysicsSystem::ApplyImpulseFromIdTo(uint32_t entity_id, mono::IBody* body)
{
    const ImpulseComponent* component = FindImpulseComponent(entity_id);
    
    const math::Matrix& transform = m_transform_system->GetTransform(entity_id);
    const float z_rotation = math::GetZRotation(transform);
    const math::Vector& impulse_vector = math::VectorFromAngle(z_rotation);

    body->ApplyLocalImpulse(impulse_vector * component->impulse_strength, math::ZeroVec);
}
