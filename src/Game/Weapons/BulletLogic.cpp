
#include "BulletLogic.h"
#include "CollisionConfiguration.h"

#include "System/Audio.h"
#include "Util/Random.h"
#include "Math/MathFunctions.h"

#include "Physics/PhysicsSystem.h"
#include "Physics/PhysicsSpace.h"

#include "System/System.h"

#include "IDebugDrawer.h"
#include "Factories.h"
#include "Rendering/Color.h"

#include <cmath>

using namespace game;

BulletLogic::BulletLogic(
    uint32_t entity_id,
    uint32_t owner_entity_id,
    const BulletConfiguration& config,
    const CollisionConfiguration& collision_config,
    mono::PhysicsSystem* physics_system)
    : m_entity_id(entity_id)
    , m_owner_entity_id(owner_entity_id)
    , m_collision_callback(collision_config.collision_callback)
{
    const float life_span = config.life_span + (mono::Random() * config.fuzzy_life_span);
    m_life_span = life_span * 1000.0f;

    if(!config.sound_file.empty())
    {
        m_sound = audio::CreateSound(config.sound_file.c_str(), audio::SoundPlayback::LOOPING);
        m_sound->Play();
    }
    else
    {
        m_sound = audio::CreateNullSound();
    }

    m_physics_system = physics_system;
    m_bullet_behaviour = config.bullet_behaviour;
    m_jumps_left = 3;
}

void BulletLogic::Update(const mono::UpdateContext& update_context)
{
    //m_sound->Position(0.0f, 0.0f);

    m_life_span -= update_context.delta_ms;
    if(m_life_span < 0)
    {
        CollisionDetails details;
        details.body = nullptr;
        details.point = math::ZeroVec;
        details.normal = math::ZeroVec;

        m_collision_callback(m_entity_id, m_owner_entity_id, BulletImpactFlag::DESTROY_THIS, details);
    }
}

mono::CollisionResolve BulletLogic::OnCollideWith(
    mono::IBody* colliding_body, const math::Vector& collision_point, const math::Vector& collision_normal, uint32_t categories)
{
    if(m_bullet_behaviour == BulletCollisionFlag::JUMPER)
    {
        const bool collide_with_static = (colliding_body->GetType() == mono::BodyType::STATIC);
        if(collide_with_static)
        {
            m_jumps_left = 0;
        }
        else if(m_jumps_left > 0)
        {
            mono::PhysicsSpace* space = m_physics_system->GetSpace();
            const uint32_t colliding_id = m_physics_system->GetIdFromBody(colliding_body);

            m_jump_ids.push_back(colliding_id);

            mono::IBody* bullet_body = m_physics_system->GetBody(m_entity_id);
            const math::Vector bullet_position = bullet_body->GetPosition();
            const math::Vector bullet_velocity_normalized = math::Normalized(bullet_body->GetVelocity());

            const mono::QueryFilter query_filter = [this, &bullet_position, &bullet_velocity_normalized](uint32_t entity_id, const math::Vector& point) {

                const bool already_processed = (std::find(m_jump_ids.begin(), m_jump_ids.end(), entity_id) != m_jump_ids.end());
                if(already_processed)
                    return false;

                const math::Vector local_point = point - bullet_position;
                const float dot_product = math::Dot(local_point, bullet_velocity_normalized);
                if(dot_product < 0.0f) // if its less than zero its behind the bullet
                    return false;

                const math::Vector normalized_local_point = math::Normalized(local_point);
                const float radians1 = math::AngleFromVector(normalized_local_point);
                const float radians2 = math::AngleFromVector(bullet_velocity_normalized);
                const float radians = std::fabs(radians1 - radians2);

                return (radians < math::ToRadians(45.0f));
            };

            const mono::QueryResult query_result = space->QueryNearest(collision_point, 2.0f, shared::CollisionCategory::ENEMY, query_filter);
            if(query_result.body)
            {
                const math::Vector found_body_position = query_result.body->GetPosition();
                const math::Vector unit_direction = math::Normalized(found_body_position - bullet_position);

                const math::Vector bullet_velocity = bullet_body->GetVelocity();
                const float bullet_velocity_length = math::Length(bullet_velocity);
                bullet_body->SetVelocity(unit_direction * bullet_velocity_length);
                m_jumps_left--;
            }
            else
            {
                m_jumps_left = 0;
            }
        }
    }

    mono::CollisionResolve resolve_type = mono::CollisionResolve::NORMAL;
    BulletImpactFlag collision_flags = BulletImpactFlag(APPLY_DAMAGE | DESTROY_THIS);

    if(m_jumps_left > 0 && (m_bullet_behaviour & BulletCollisionFlag::JUMPER))
    {
        resolve_type = mono::CollisionResolve::IGNORE;
        collision_flags = BulletImpactFlag(collision_flags & ~DESTROY_THIS);
    }

    CollisionDetails details;
    details.body = colliding_body;
    details.point = collision_point;
    details.normal = collision_normal;

    m_collision_callback(m_entity_id, m_owner_entity_id, collision_flags, details);
    return resolve_type;
}

void BulletLogic::OnSeparateFrom(mono::IBody* body)
{

}
