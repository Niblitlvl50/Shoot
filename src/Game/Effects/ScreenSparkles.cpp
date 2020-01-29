
#include "ScreenSparkles.h"
#include "Particle/ParticleSystem.h"
#include "Rendering/Texture/TextureFactory.h"
#include "Util/Random.h"

#include "Math/MathFunctions.h"
#include "Math/Quad.h"
#include "Factories.h"
#include "Entity/IEntityManager.h"

using namespace game;

namespace
{
    void Generator(const math::Vector& position, mono::ParticlePoolComponent& pool, size_t index, const math::Quad& viewport)
    {
        const float half_height = viewport.mB.y / 2.0f;

        const float x = 0.0f;
        const float y = mono::Random(-half_height, half_height);

        const float velocity_x = mono::Random(-5.0f, -3.0f);
        const float velocity_y = mono::Random(-1.0f, 1.0f);

        const float sign = mono::RandomInt(0, 1) == 0 ? 1.0f : -1.0f;
        const float angular_velocity = mono::Random(0.3f, 1.0f) * sign;

        const int life = mono::RandomInt(0, 500) + 5000;

        pool.position[index] = position + math::Vector(x, y);
        pool.velocity[index] = math::Vector(velocity_x, velocity_y);
        pool.rotation[index] = mono::Random(0.0f, math::PI() * 2.0f);
        pool.angular_velocity[index] = angular_velocity;
        pool.start_color[index] = mono::Color::RGBA(1.0f, 0.0f, 0.0f, 1.0f);
        pool.end_color[index] = mono::Color::RGBA(0.0f, 1.0f, 0.0f, 1.0f);
        pool.start_size[index] = mono::Random(58.0f, 76.0f);
        pool.end_size[index] = mono::Random(2.0f, 6.0f);
        pool.start_life[index] = life;
        pool.life[index] = life;
    }

    void SparklesUpdater(mono::ParticlePoolComponent& pool, size_t count, unsigned int delta)
    {
        const float float_delta = float(delta) / 1000.0f;

        for(size_t index = 0; index < count; ++index)
        {
            const float t = 1.0f - float(pool.life[index]) / float(pool.start_life[index]);

            pool.position[index] += pool.velocity[index] * float_delta;
            pool.color[index] = mono::Color::Lerp(pool.start_color[index], pool.end_color[index], t);
            pool.size[index] = (1.0f - t) * pool.start_size[index] + t * pool.end_size[index];
            pool.rotation[index] += pool.angular_velocity[index] * float_delta;
        }
    }
}

ScreenSparkles::ScreenSparkles(mono::ParticleSystem* particle_system, const math::Quad& viewport)
    : m_particle_system(particle_system)
{
    const float x = viewport.mB.x;
    const float y = viewport.mB.y / 2.0f;

    mono::Entity sparkles_entity = g_entity_manager->CreateEntity("screensparkles", {});
    particle_system->AllocatePool(sparkles_entity.id, 500, SparklesUpdater);

    const mono::ITexturePtr texture = mono::CreateTexture("res/textures/x4.png");
    particle_system->SetPoolDrawData(sparkles_entity.id, texture, mono::BlendMode::SOURCE_ALPHA);

    const auto generator_proxy = [viewport](const math::Vector& position, mono::ParticlePoolComponent& pool, size_t index) {
        Generator(position, pool, index, viewport);
    };
    particle_system->AttachEmitter(sparkles_entity.id, math::Vector(x, y), -1.0f, 100.0f, mono::EmitterType::CONTINOUS, generator_proxy);

    m_particle_entity = sparkles_entity.id;
}

ScreenSparkles::~ScreenSparkles()
{
    m_particle_system->ReleasePool(m_particle_entity);
    g_entity_manager->ReleaseEntity(m_particle_entity);
}
