
#include "Shockwave.h"

#include "CollisionConfiguration.h"
#include "DamageSystem/DamageSystem.h"
#include "Weapons/WeaponTypes.h"

#include "Physics/PhysicsSystem.h"
#include "Physics/PhysicsSpace.h"

void game::ShockwaveAt(
    mono::PhysicsSystem* physics_system,
    const math::Vector& world_position,
    float shockwave_radius,
    float magnitude)
{
    ShockwaveAndDamageAt(physics_system, nullptr, world_position, shockwave_radius, magnitude, 0, 0, CollisionCategory::CC_ALL);
}

void game::ShockwaveAtForTypes(
    mono::PhysicsSystem* physics_system,
    const math::Vector& world_position,
    float shockwave_radius,
    float magnitude,
    uint32_t object_types)
{
    ShockwaveAndDamageAt(physics_system, nullptr, world_position, shockwave_radius, magnitude, 0, 0, object_types);
}

void game::ShockwaveAndDamageAt(
    mono::PhysicsSystem* physics_system,
    game::DamageSystem* damage_system,
    const math::Vector& world_position,
    float shockwave_radius,
    float magnitude,
    int damage,
    uint32_t who_did_damage,
    uint32_t object_types)
{
    const std::vector<mono::QueryResult> found_bodies =
        physics_system->GetSpace()->QueryRadius(world_position, shockwave_radius, object_types);

    for(const mono::QueryResult& query_result : found_bodies)
    {
        const uint32_t other_entity_id = physics_system->GetIdFromBody(query_result.body);
        const math::Vector body_position = query_result.body->GetPosition();

        const math::Vector delta = body_position - world_position;
        const math::Vector normalized_delta = math::Normalized(delta);

        //const float length = math::Length(delta); // Maybe scale the damage and impulse with the length

        query_result.body->ApplyImpulse(normalized_delta * magnitude, world_position);

        if(damage_system)
            damage_system->ApplyDamage(other_entity_id, who_did_damage, NO_WEAPON_IDENTIFIER, DamageDetails(damage, false));
    }
}
