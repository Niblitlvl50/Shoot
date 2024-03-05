
#include "WeaponEntityFactory.h"
#include "WeaponConfiguration.h"
#include "BulletWeapon/BulletLogic.h"
#include "BulletWeapon/WebberLogic.h"
#include "Entity/EntityLogicSystem.h"

#include "EntitySystem/IEntityManager.h"
#include "Math/Matrix.h"
#include "TransformSystem/TransformSystem.h"
#include "Physics/PhysicsSystem.h"
#include "Physics/PhysicsSpace.h"
#include "Entity/Component.h"

#include <string>

using namespace game;

WeaponEntityFactory::WeaponEntityFactory(
    mono::IEntityManager* entity_manager,
    mono::TransformSystem* transform_system,
    mono::PhysicsSystem* physics_system,
    game::EntityLogicSystem* logic_system,
    game::TargetSystem* target_system)
    : m_entity_manager(entity_manager)
    , m_transform_system(transform_system)
    , m_physics_system(physics_system)
    , m_logic_system(logic_system)
    , m_target_system(target_system)
{ }

mono::Entity WeaponEntityFactory::CreateBulletEntity(
    uint32_t owner_id,
    const BulletConfiguration& bullet_config,
    const CollisionConfiguration& collision_config,
    const math::Vector& target,
    const math::Vector& velocity,
    float bullet_direction,
    const math::Matrix& transform) const
{
    mono::Entity bullet_entity = m_entity_manager->SpawnEntity(bullet_config.entity_file.c_str());
    m_transform_system->SetTransform(bullet_entity.id, transform, mono::TransformState::CLIENT);

    IEntityLogic* bullet_logic = new BulletLogic(
        bullet_entity.id,
        owner_id,
        target,
        velocity,
        bullet_direction,
        bullet_config,
        collision_config,
        m_transform_system,
        m_physics_system,
        m_target_system);

    m_entity_manager->AddComponent(bullet_entity.id, BEHAVIOUR_COMPONENT);
    m_logic_system->AddLogic(bullet_entity.id, bullet_logic);

    return bullet_entity;
}

mono::Entity WeaponEntityFactory::CreateWebberEntity(const std::vector<mono::QueryResult>& bodies, const math::Matrix& transform) const
{
    mono::Entity entity = m_entity_manager->CreateEntity("webber", {});
    m_entity_manager->AddComponent(entity.id, PHYSICS_COMPONENT);
    m_entity_manager->AddComponent(entity.id, BEHAVIOUR_COMPONENT);

    mono::IBody* body = m_physics_system->GetBody(entity.id);
    body->SetType(mono::BodyType::STATIC);

    m_transform_system->SetTransform(entity.id, transform, mono::TransformState::CLIENT);
 
    WebberLogic* webber_logic = new WebberLogic(entity.id, m_entity_manager, m_physics_system, m_transform_system);
    for(const mono::QueryResult& query_result : bodies)
        webber_logic->AttachTo(query_result.body, query_result.point);

    m_logic_system->AddLogic(entity.id, webber_logic);

    return entity;
}
