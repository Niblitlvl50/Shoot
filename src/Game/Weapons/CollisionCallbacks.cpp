
#include "CollisionCallbacks.h"

#include "DamageSystem/DamageSystem.h"
#include "GameCamera/CameraSystem.h"
#include "Camera/ICamera.h"
#include "Effects/DamageEffect.h"
#include "Effects/ImpactEffect.h"

#include "EntitySystem/Entity.h"
#include "EntitySystem/IEntityManager.h"
#include "Math/MathFwd.h"
#include "Math/MathFunctions.h"
#include "Particle/ParticleSystem.h"
#include "Physics/IBody.h"
#include "Physics/PhysicsSystem.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Sprite/Sprite.h"
#include "SystemContext.h"
#include "TransformSystem/TransformSystem.h"

namespace
{
    game::DamageEffect* g_damage_effect = nullptr;
    game::ImpactEffect* g_impact_effect = nullptr;
}

void game::InitWeaponCallbacks(mono::SystemContext* system_context)
{
    mono::ParticleSystem* particle_system = system_context->GetSystem<mono::ParticleSystem>();
    mono::IEntityManager* entity_system = system_context->GetSystem<mono::IEntityManager>();

    g_damage_effect = new game::DamageEffect(particle_system, entity_system);
    g_impact_effect = new game::ImpactEffect(particle_system, entity_system);
}

void game::CleanupWeaponCallbacks()
{
    delete g_damage_effect;
    g_damage_effect = nullptr;

    delete g_impact_effect;
    g_impact_effect = nullptr;
}

uint32_t game::SpawnEntityWithAnimation(
    const char* entity_file,
    int animation_id,
    uint32_t position_at_transform_id,
    mono::IEntityManager* entity_manager,
    mono::TransformSystem* transform_system,
    mono::SpriteSystem* sprite_system)
{
    const mono::Entity spawned_entity = entity_manager->SpawnEntity(entity_file);
    math::Matrix& entity_transform = transform_system->GetTransform(spawned_entity.id);
    entity_transform = transform_system->GetWorld(position_at_transform_id);

    mono::Sprite* spawned_entity_sprite = sprite_system->GetSprite(spawned_entity.id);

    const auto remove_entity_callback = [entity_manager](uint32_t sprite_id) {
        entity_manager->ReleaseEntity(sprite_id);
    };
    spawned_entity_sprite->SetAnimation(animation_id, remove_entity_callback);

    return spawned_entity.id;
}

void game::StandardCollision(
    uint32_t entity_id,
    uint32_t owner_entity_id,
    int damage,
    game::BulletImpactFlag flags,
    const CollisionDetails& details,
    mono::IEntityManager* entity_manager,
    game::DamageSystem* damage_system,
    mono::PhysicsSystem* physics_system,
    mono::SpriteSystem* sprite_system,
    mono::TransformSystem* transform_system)
{
    bool did_damage = false;

    const uint32_t other_entity_id = physics_system->GetIdFromBody(details.body);
    if(other_entity_id != mono::INVALID_ID && (flags & game::BulletImpactFlag::APPLY_DAMAGE))
    {
        const DamageResult result = damage_system->ApplyDamage(other_entity_id, damage, owner_entity_id);
        did_damage = result.did_damage;
    }

    if(details.body)
    {
        const float direction = math::AngleFromVector(details.normal);
        if(did_damage)
        {
            g_damage_effect->EmitGibsAt(details.point, direction);
            mono::Sprite* sprite = sprite_system->GetSprite(other_entity_id);
            sprite->FlashSprite();
        }
        else
        {
            g_impact_effect->EmittAt(details.point, direction);
        }
    }

    if(flags & game::BulletImpactFlag::DESTROY_THIS)
        entity_manager->ReleaseEntity(entity_id);
}

void game::PlasmaCollision(
    uint32_t entity_id,
    uint32_t owner_entity_id,
    int damage,
    game::BulletImpactFlag flags,
    const CollisionDetails& details,
    mono::IEntityManager* entity_manager,
    game::DamageSystem* damage_system,
    mono::PhysicsSystem* physics_system,
    mono::SpriteSystem* sprite_system,
    mono::TransformSystem* transform_system)
{
    StandardCollision(
        entity_id, owner_entity_id, damage, flags, details, entity_manager, damage_system, physics_system, sprite_system, transform_system);
}

void game::RocketCollision(
    uint32_t entity_id,
    uint32_t owner_entity_id,
    int damage,
    game::BulletImpactFlag flags,
    const CollisionDetails& details,
    mono::IEntityManager* entity_manager,
    game::DamageSystem* damage_system,
    game::CameraSystem* camera_system,
    mono::PhysicsSystem* physics_system,
    mono::SpriteSystem* sprite_system,
    mono::TransformSystem* transform_system)
{
    StandardCollision(
        entity_id, owner_entity_id, damage, flags, details, entity_manager, damage_system, physics_system, sprite_system, transform_system);

    if(details.body)
        SpawnEntityWithAnimation("res/entities/explosion_rocket.entity", 0, entity_id, entity_manager, transform_system, sprite_system);

    const mono::ICamera* camera = camera_system->GetActiveCamera();
    const bool collision_visible = math::PointInsideQuad(details.point, camera->GetViewport());
    if(collision_visible)
        camera_system->AddCameraShake(0.1f);
}

void game::CacoPlasmaCollision(
    uint32_t entity_id,
    uint32_t owner_entity_id,
    int damage,
    game::BulletImpactFlag flags,
    const CollisionDetails& details,
    mono::IEntityManager* entity_manager,
    game::DamageSystem* damage_system,
    mono::PhysicsSystem* physics_system,
    mono::SpriteSystem* sprite_system,
    mono::TransformSystem* transform_system)
{
    StandardCollision(
        entity_id, owner_entity_id, damage, flags, details, entity_manager, damage_system, physics_system, sprite_system, transform_system);

    if(details.body)
        SpawnEntityWithAnimation("res/entities/explosion_caco.entity", 0, entity_id, entity_manager, transform_system, sprite_system);
}
