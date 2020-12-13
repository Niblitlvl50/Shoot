
#include "CollisionCallbacks.h"

#include "DamageSystem.h"

#include "EntitySystem/IEntityManager.h"
#include "Math/MathFwd.h"
#include "Math/MathFunctions.h"
#include "Physics/IBody.h"
#include "Physics/PhysicsSystem.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Sprite/Sprite.h"
#include "TransformSystem/TransformSystem.h"

void game::SpawnEntityWithAnimation(
    const char* entity_file,
    int animation_id,
    uint32_t position_at_transform_id,
    mono::IEntityManager* entity_manager,
    mono::TransformSystem* transform_system,
    mono::SpriteSystem* sprite_system)
{
    const mono::Entity spawned_entity = entity_manager->CreateEntity(entity_file);
    math::Matrix& entity_transform = transform_system->GetTransform(spawned_entity.id);
    entity_transform = transform_system->GetWorld(position_at_transform_id);

    mono::Sprite* spawned_entity_sprite = sprite_system->GetSprite(spawned_entity.id);

    const auto remove_entity_callback = [spawned_entity, entity_manager]() {
        entity_manager->ReleaseEntity(spawned_entity.id);
    };
    spawned_entity_sprite->SetAnimation(animation_id, remove_entity_callback);
}

void game::StandardCollision(
    uint32_t entity_id,
    uint32_t owner_entity_id,
    game::BulletCollisionFlag flags,
    const mono::IBody* other,
    mono::IEntityManager* entity_manager,
    game::DamageSystem* damage_system,
    mono::PhysicsSystem* physics_system)
{
    const uint32_t other_entity_id = physics_system->GetIdFromBody(other);
    if(other_entity_id != std::numeric_limits<uint32_t>::max() && flags & game::BulletCollisionFlag::APPLY_DAMAGE)
        damage_system->ApplyDamage(other_entity_id, 20, owner_entity_id);

    if(flags & game::BulletCollisionFlag::DESTROY_THIS)
        entity_manager->ReleaseEntity(entity_id);
}

void game::RocketCollision(
    uint32_t entity_id,
    uint32_t owner_entity_id,
    game::BulletCollisionFlag flags, 
    const mono::IBody* other,
    mono::IEntityManager* entity_manager,
    game::DamageSystem* damage_system,
    mono::PhysicsSystem* physics_system,
    mono::SpriteSystem* sprite_system,
    mono::TransformSystem* transform_system)
{
    StandardCollision(entity_id, owner_entity_id, flags, other, entity_manager, damage_system, physics_system);
    //SpawnEntityWithAnimation("res/entities/explosion.entity", 0, entity_id, entity_manager, transform_system, sprite_system);
    //event_handler.DispatchEvent(game::ShockwaveEvent(explosion_config.position, 150));
}

void game::CacoPlasmaCollision(
    uint32_t entity_id,
    uint32_t owner_entity_id,
    game::BulletCollisionFlag flags,
    const mono::IBody* other,
    mono::IEntityManager* entity_manager,
    game::DamageSystem* damage_system,
    mono::PhysicsSystem* physics_system,
    mono::SpriteSystem* sprite_system,
    mono::TransformSystem* transform_system)
{
    StandardCollision(entity_id, owner_entity_id, flags, other, entity_manager, damage_system, physics_system);
    SpawnEntityWithAnimation("res/entities/caco_explosion.entity", 0, entity_id, entity_manager, transform_system, sprite_system);
}
