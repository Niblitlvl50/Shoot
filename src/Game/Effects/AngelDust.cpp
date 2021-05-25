
#include "AngelDust.h"
#include "Particle/ParticleSystem.h"
#include "Rendering/RenderSystem.h"
#include "Rendering/Texture/ITextureFactory.h"
#include "Util/Random.h"

#include "Math/MathFunctions.h"
#include "Math/Quad.h"
#include "Factories.h"
#include "EntitySystem/IEntityManager.h"

using namespace game;

namespace
{
    void Generator(const math::Vector& position, mono::ParticlePoolComponent& pool, size_t index, const math::Quad& area)
    {
        const float x = mono::Random(area.mA.x, area.mB.x);
        const float y = mono::Random(area.mA.y, area.mB.y);

        const float velocity_x = mono::Random(-1.0f, 1.0f);
        const float velocity_y = mono::Random(-0.5f, 0.5f);

        const int life = mono::RandomInt(0, 500) + 5000;

        pool.position[index] = position + math::Vector(x, y);
        pool.velocity[index] = math::Vector(velocity_x, velocity_y);
        //pool.rotation[index] = mono::Random(0.0f, math::PI() * 2.0f);
        //pool.angular_velocity[index] = angular_velocity;
        pool.start_color[index] = mono::Color::RGBA(1.0f, 1.0f, 1.0f, 0.5f);
        pool.end_color[index] = mono::Color::RGBA(1.0f, 1.0f, 1.0f, 0.0f);
        pool.start_size[index] = 32.0f; //mono::Random(32.0f, 76.0f);
        pool.end_size[index] = 4.0f; //mono::Random(2.0f, 6.0f);
        pool.start_life[index] = life;
        pool.life[index] = life;
    }

    void SparklesUpdater(mono::ParticlePoolComponent& pool, size_t count, uint32_t delta_ms)
    {
        const float delta_seconds = float(delta_ms) / 1000.0f;

        for(size_t index = 0; index < count; ++index)
        {
            const float t = 1.0f - float(pool.life[index]) / float(pool.start_life[index]);

            pool.position[index] += pool.velocity[index] * delta_seconds;
            pool.color[index] = mono::Color::Lerp(pool.start_color[index], pool.end_color[index], t);
            pool.size[index] = (1.0f - t) * pool.start_size[index] + t * pool.end_size[index];
            pool.rotation[index] += pool.angular_velocity[index] * delta_seconds;
        }
    }
}

AngelDust::AngelDust(
    mono::ParticleSystem* particle_system, mono::IEntityManager* entity_system, const math::Quad& area)
    : m_particle_system(particle_system)
    , m_entity_system(entity_system)
{
    mono::Entity sparkles_entity = m_entity_system->CreateEntity("angeldust", {});
    particle_system->AllocatePool(sparkles_entity.id, 1000, SparklesUpdater);

    const mono::ITexturePtr texture = mono::GetTextureFactory()->CreateTexture("res/textures/particles/flare.png");
    particle_system->SetPoolDrawData(sparkles_entity.id, texture, mono::BlendMode::ONE);

    const auto generator_proxy = [area](const math::Vector& position, mono::ParticlePoolComponent& pool, size_t index) {
        Generator(position, pool, index, area);
    };
    particle_system->AttachEmitter(sparkles_entity.id, math::ZeroVec, -1.0f, 200.0f, mono::EmitterType::CONTINOUS, generator_proxy);

    m_particle_entity = sparkles_entity.id;
}

AngelDust::~AngelDust()
{
    m_particle_system->ReleasePool(m_particle_entity);
    m_entity_system->ReleaseEntity(m_particle_entity);
}
