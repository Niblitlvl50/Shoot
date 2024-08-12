
#include "CollisionCallbacks.h"
#include "WeaponEntityFactory.h"

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
#include "Physics/PhysicsSpace.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Sprite/Sprite.h"
#include "SystemContext.h"
#include "TransformSystem/TransformSystem.h"
#include "Util/Random.h"

#include "System/Audio.h"

namespace
{
    game::DamageEffect* g_damage_effect = nullptr;
    game::ImpactEffect* g_impact_effect = nullptr;

    std::vector<audio::ISoundPtr> g_death_sounds; 
}

void game::InitWeaponCallbacks(mono::SystemContext* system_context)
{
    mono::ParticleSystem* particle_system = system_context->GetSystem<mono::ParticleSystem>();
    mono::IEntityManager* entity_system = system_context->GetSystem<mono::IEntityManager>();

    g_damage_effect = new game::DamageEffect(particle_system, entity_system);
    g_impact_effect = new game::ImpactEffect(particle_system, entity_system);

    g_death_sounds.push_back(audio::CreateSound("res/sound/death/death_squish01.wav", audio::SoundPlayback::ONCE));
    g_death_sounds.push_back(audio::CreateSound("res/sound/death/death_squish02.wav", audio::SoundPlayback::ONCE));
    g_death_sounds.push_back(audio::CreateSound("res/sound/death/death_squish03.wav", audio::SoundPlayback::ONCE));
    g_death_sounds.push_back(audio::CreateSound("res/sound/death/death_squish04.wav", audio::SoundPlayback::ONCE));
    g_death_sounds.push_back(audio::CreateSound("res/sound/death/death_squish05.wav", audio::SoundPlayback::ONCE));
    g_death_sounds.push_back(audio::CreateSound("res/sound/death/death_squish06.wav", audio::SoundPlayback::ONCE));
    g_death_sounds.push_back(audio::CreateSound("res/sound/death/death_squish07.wav", audio::SoundPlayback::ONCE));
    g_death_sounds.push_back(audio::CreateSound("res/sound/death/death_squish08.wav", audio::SoundPlayback::ONCE));
    g_death_sounds.push_back(audio::CreateSound("res/sound/death/death_squish09.wav", audio::SoundPlayback::ONCE));
    g_death_sounds.push_back(audio::CreateSound("res/sound/death/death_squish10.wav", audio::SoundPlayback::ONCE));
}

void game::CleanupWeaponCallbacks()
{
    delete g_damage_effect;
    g_damage_effect = nullptr;

    delete g_impact_effect;
    g_impact_effect = nullptr;

    g_death_sounds.clear();
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

    if(sprite_system->IsAllocated(spawned_entity.id))
    {
        mono::Sprite* spawned_entity_sprite = sprite_system->GetSprite(spawned_entity.id);

        const auto remove_entity_callback = [entity_manager](uint32_t sprite_id) {
            entity_manager->ReleaseEntity(sprite_id);
        };
        spawned_entity_sprite->SetAnimation(animation_id, remove_entity_callback);
    }

    return spawned_entity.id;
}

void game::StandardCollision(
    uint32_t entity_id,
    uint32_t owner_entity_id,
    uint32_t weapon_identifier_hash,
    int damage,
    const char* impact_entity,
    game::BulletImpactFlag flags,
    const CollisionDetails& collision_details,
    mono::IEntityManager* entity_manager,
    game::DamageSystem* damage_system,
    mono::SpriteSystem* sprite_system,
    mono::TransformSystem* transform_system)
{
    if(collision_details.body)
    {
        bool did_damage = false;
        const uint32_t other_entity_id = collision_details.body->GetId();

        if(flags & game::BulletImpactFlag::APPLY_DAMAGE)
        {
            const DamageResult result = damage_system->ApplyDamage(other_entity_id, owner_entity_id, weapon_identifier_hash, damage);
            did_damage = result.did_damage;

            if(result.did_damage && result.health_left <= 0)
            {
                const int index = mono::RandomInt(0, g_death_sounds.size() - 1);
                g_death_sounds[index]->Play();
            }
        }

        if(impact_entity != nullptr)
        {
            SpawnEntityWithAnimation(impact_entity, 0, entity_id, entity_manager, transform_system, sprite_system);
        }
        else
        {
            const float direction = math::AngleFromVector(collision_details.normal);
            if(did_damage)
                g_damage_effect->EmitGibsAt(collision_details.point, direction);
            else
                g_impact_effect->EmittAt(collision_details.point, direction);
        }

        if(did_damage)
        {
            mono::Sprite* sprite = sprite_system->GetSprite(other_entity_id);
            sprite->FlashSprite();
        }
    }

    if(flags & game::BulletImpactFlag::DESTROY_THIS)
        entity_manager->ReleaseEntity(entity_id);
}

void game::RocketCollision(
    uint32_t entity_id,
    uint32_t owner_entity_id,
    uint32_t weapon_identifier_hash,
    int damage,
    const char* impact_entity,
    game::BulletImpactFlag flags,
    const CollisionDetails& collision_details,
    mono::IEntityManager* entity_manager,
    game::DamageSystem* damage_system,
    game::CameraSystem* camera_system,
    mono::SpriteSystem* sprite_system,
    mono::TransformSystem* transform_system)
{
    StandardCollision(
        entity_id, owner_entity_id, weapon_identifier_hash, damage, impact_entity, flags, collision_details, entity_manager, damage_system, sprite_system, transform_system);

    const mono::ICamera* camera = camera_system->GetActiveCamera();
    const bool collision_visible = math::PointInsideQuad(collision_details.point, camera->GetViewport());
    if(collision_visible)
        camera_system->AddCameraShake(0.1f);
}

void game::WebberCollision(
    uint32_t entity_id,
    uint32_t owner_entity_id,
    uint32_t weapon_identifier_hash,
    int damage,
    const char* impact_entity,
    game::BulletImpactFlag flags,
    const game::CollisionDetails& collision_details,
    mono::IEntityManager* entity_manager,
    game::DamageSystem* damage_system,
    mono::PhysicsSystem* physics_system,
    mono::SpriteSystem* sprite_system,
    mono::TransformSystem* transform_system,
    const game::WeaponEntityFactory* entity_factory)
{
    StandardCollision(
        entity_id, owner_entity_id, weapon_identifier_hash, damage, impact_entity, flags, collision_details, entity_manager, damage_system, sprite_system, transform_system);

    const std::vector<mono::QueryResult> found_bodies =
        physics_system->GetSpace()->QueryRadius(collision_details.point, 2.5f, game::CollisionCategory::ENEMY);
    if(!found_bodies.empty())
    {
        entity_factory->CreateWebberEntity(found_bodies, math::CreateMatrixWithPosition(collision_details.point));
    }
}
