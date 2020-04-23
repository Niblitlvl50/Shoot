
#include "BulletLogic.h"
#include "CollisionConfiguration.h"

#include "Audio/ISound.h"
#include "Audio/AudioFactory.h"
#include "Util/Random.h"
#include "Math/MathFunctions.h"

#include "Physics/PhysicsSystem.h"
#include "Physics/CMSpace.h"

#include "System/System.h"

#include "IDebugDrawer.h"
#include "Factories.h"
#include "Rendering/Color.h"

#include <cmath>

using namespace game;

BulletLogic::BulletLogic(uint32_t entity_id, const BulletConfiguration& config, mono::PhysicsSystem* physics_system)
    : m_entity_id(entity_id)
    , m_collision_callback(config.collision_callback)
{
    const float life_span = config.life_span + (mono::Random() * config.fuzzy_life_span);
    m_life_span = life_span * 1000.0f;

    if(config.sound_file)
    {
        m_sound = mono::AudioFactory::CreateSound(config.sound_file, mono::SoundPlayback::LOOPING, mono::SoundPosition::GLOBAL);
        m_sound->Play();
    }
    else
    {
        m_sound = mono::AudioFactory::CreateNullSound();
    }

    m_physics_system = physics_system;
    m_bullet_behaviour = config.bullet_behaviour;
    m_jumps_left = 3;
}

void BulletLogic::Update(uint32_t delta_ms)
{
    m_sound->Position(0.0f, 0.0f);

    m_life_span -= delta_ms;
    if(m_life_span < 0)
        m_collision_callback(m_entity_id, BulletCollisionFlag::DESTROY_THIS, nullptr);
}

mono::CollisionResolve BulletLogic::OnCollideWith(mono::IBody* colliding_body, const math::Vector& collision_point, uint32_t categories)
{
    if(m_bullet_behaviour == BulletCollisionBehaviour::JUMPER)
    {
        const bool collide_with_static = colliding_body->GetType() == mono::BodyType::STATIC;
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
            const math::Vector bullet_velocity = bullet_body->GetVelocity();
            const math::Vector bullet_velocity_normalized = math::Normalize(bullet_body->GetVelocity());
            const float bullet_velocity_length = math::Length(bullet_velocity);

            const mono::QueryFilter query_filter = [this, &bullet_position, &bullet_velocity_normalized](uint32_t entity_id, const math::Vector& point) {

                const bool already_processed = (std::find(m_jump_ids.begin(), m_jump_ids.end(), entity_id) != m_jump_ids.end());
                if(already_processed)
                    return false;

                const math::Vector local_point = point - bullet_position;
                const float dot_product = math::Dot(local_point, bullet_velocity_normalized);
                if(dot_product < 0.0f) // if its less than zero its behind the bullet
                    return false;

                const float radians = math::AngleBetweenPoints(local_point, bullet_velocity_normalized);
                const float degrees = math::ToDegrees(radians);
                printf("%.2f\n", degrees);

                const math::Vector normalized_local_point = math::Normalize(local_point);
                g_debug_drawer->DrawLine(point, point + bullet_velocity_normalized, 2.0f, mono::Color::RED);
                g_debug_drawer->DrawLine(point, point + normalized_local_point, 2.0f, mono::Color::GREEN);

                //const float base_radians = math::AngleFromVector(bullet_velocity_normalized);
                //const float radians = math::AngleBetweenPoints(local_point, bullet_velocity_normalized);
                //const float degrees = math::ToDegrees(base_radians - radians);
                //printf("base angle %.2f, angle %.2f, calculated %.2f\n", math::ToDegrees(base_radians), math::ToDegrees(radians), degrees);

                return (std::fabs(degrees) < 45.0f);
            };

            printf("\n");

            const mono::IBody* found_body = space->QueryNearest(collision_point, 10.0f, game::CollisionCategory::ENEMY, query_filter);
            if(found_body)
            {
                const math::Vector found_body_position = found_body->GetPosition();
                const math::Vector unit_direction = math::Normalize(found_body_position - bullet_position);

                bullet_body->SetVelocity(unit_direction * bullet_velocity_length);
                m_jumps_left--;
            }
        }
    }

    mono::CollisionResolve resolve_type = mono::CollisionResolve::NORMAL;
    BulletCollisionFlag collision_flags =
        BulletCollisionFlag(BulletCollisionFlag::APPLY_DAMAGE | BulletCollisionFlag::DESTROY_THIS);

    if(m_jumps_left > 0 && m_bullet_behaviour == BulletCollisionBehaviour::JUMPER)
    {
        resolve_type = mono::CollisionResolve::IGNORE;
        collision_flags = BulletCollisionFlag(collision_flags & ~BulletCollisionFlag::DESTROY_THIS);
    }

    m_collision_callback(m_entity_id, collision_flags, colliding_body);
    return resolve_type;
}
