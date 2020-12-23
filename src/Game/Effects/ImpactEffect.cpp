
#include "ImpactEffect.h"

#include "Particle/ParticleSystem.h"
#include "Rendering/Texture/ITextureFactory.h"
#include "Util/Random.h"

#include "Math/MathFunctions.h"
#include "Math/EasingFunctions.h"
#include "Factories.h"
#include "EntitySystem/IEntityManager.h"

using namespace game;

namespace
{
    void GibsGenerator(const math::Vector& position, mono::ParticlePoolComponent& pool, size_t index, float direction)
    {
        constexpr float ten_degrees = math::ToRadians(45.0f);

        const float direction_variation = mono::Random(-ten_degrees, ten_degrees);
        const math::Vector& velocity = math::VectorFromAngle(direction + direction_variation);

        const int life = mono::RandomInt(100, 150);
        const float velocity_variation = mono::Random(10.0f, 16.0f);
        const float size = mono::Random(16.0f, 20.0f);

        pool.position[index] = position;
        pool.velocity[index] = velocity * velocity_variation;

        pool.rotation[index] = 0.0f;
        pool.angular_velocity[index] = 10.0f;

        pool.color[index] = mono::Color::BLACK;
        pool.start_color[index] = mono::Color::BLACK;
        pool.end_color[index] = mono::Color::BLACK;

        pool.size[index] = size;
        pool.start_size[index] = size;
        pool.end_size[index] = size;

        pool.start_life[index] = life;
        pool.life[index] = life;
    }

    void GibsUpdater(mono::ParticlePoolComponent& pool, size_t count, uint32_t delta_ms)
    {
        const float float_delta = float(delta_ms) / 1000.0f;

        for(size_t index = 0; index < count; ++index)
        {
            const float t = 1.0f - float(pool.life[index]) / float(pool.start_life[index]);

            pool.velocity[index] *= 0.90;
            pool.position[index] += pool.velocity[index] * float_delta;
            pool.rotation[index] += pool.angular_velocity[index] * float_delta;
            pool.size[index] = (1.0f - t) * pool.start_size[index] + t * pool.end_size[index];
            pool.color[index] = mono::Color::LerpRGB(pool.start_color[index], pool.end_color[index], t);
        }
    }    
}

ImpactEffect::ImpactEffect(mono::ParticleSystem* particle_system)
    : m_particle_system(particle_system)
{
    mono::Entity particle_entity = g_entity_manager->CreateEntity("impacteffect", {});
    particle_system->AllocatePool(particle_entity.id, 50, GibsUpdater);

    const mono::ITexturePtr texture = mono::GetTextureFactory()->CreateTexture("res/textures/particles/white_square.png");
    particle_system->SetPoolDrawData(particle_entity.id, texture, mono::BlendMode::SOURCE_ALPHA);

    m_particle_entity = particle_entity.id;
}

ImpactEffect::~ImpactEffect()
{
    m_particle_system->ReleasePool(m_particle_entity);
    g_entity_manager->ReleaseEntity(m_particle_entity);
}

void ImpactEffect::EmittAt(const math::Vector& position, float direction)
{
    const auto generator_proxy = [direction](const math::Vector& position, mono::ParticlePoolComponent& pool, size_t index) {
        GibsGenerator(position, pool, index, direction);
    };

    m_particle_system->AttachEmitter(
        m_particle_entity, position, 0.5f, 30.0f, mono::EmitterType::BURST_REMOVE_ON_FINISH, generator_proxy);
}

