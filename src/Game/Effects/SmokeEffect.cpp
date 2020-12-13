
#include "SmokeEffect.h"

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
    void SmokeGenerator(const math::Vector& position, mono::ParticlePoolComponent& pool, size_t index)
    {
        const bool go_left = (index % 2) == 0;
        const math::Vector velocity = go_left ? math::Vector(-1.0f, 0.0f) : math::Vector(1.0f, 0.0f);

        const float x_variation = mono::Random(-0.2f, 0.2f);
        const float y_variation = mono::Random(-0.2f, 0.2f);
        const float velocity_variation = mono::Random(1.0f, 3.0f);
        const float size = mono::Random(48.0f, 64.0f);
        const float end_size = mono::Random(64.0f, 80.0f);
        const int life = mono::RandomInt(200, 400);

        pool.position[index] = position + math::Vector(x_variation, y_variation);
        pool.rotation[index] = 0.0f;
        pool.velocity[index] = velocity * velocity_variation;
        //pool.angular_velocity[index] = mono::Random(-1.1f, 1.1f);
        pool.start_color[index] = mono::Color::OFF_WHITE;
        pool.end_color[index] = mono::Color::RGBA(1.0f, 1.0f, 1.0f, 0.0f);
        pool.start_size[index] = size;
        pool.end_size[index] = end_size;
        pool.size[index] = size;
        pool.start_life[index] = life;
        pool.life[index] = life;
    }

    void GibsUpdater(mono::ParticlePoolComponent& pool, size_t count, uint32_t delta_ms)
    {
        const float delta_seconds = float(delta_ms) / 1000.0f;

        for(size_t index = 0; index < count; ++index)
        {
            const float t = 1.0f - float(pool.life[index]) / float(pool.start_life[index]);
            const float t2 = float(pool.start_life[index]) - float(pool.life[index]);
            const float duration = float(pool.start_life[index]);

            pool.velocity[index] *= 0.90;
            pool.position[index] += pool.velocity[index] * delta_seconds;
            //pool.rotation[index] += pool.angular_velocity[index] * delta_seconds;
            pool.size[index] = (1.0f - t) * pool.start_size[index] + t * pool.end_size[index];

            const float alpha1 = pool.start_color[index].alpha;
            const float alpha2 = pool.end_color[index].alpha;
            const float delta_alpha = alpha2 - alpha1;

            pool.color[index] = mono::Color::LerpRGB(pool.start_color[index], pool.end_color[index], t);
            pool.color[index].alpha = math::EaseInCubic(t2, duration, alpha1, delta_alpha);
        }
    }
}

SmokeEffect::SmokeEffect(mono::ParticleSystem* particle_system)
    : m_particle_system(particle_system)
{
    mono::Entity particle_entity = g_entity_manager->CreateEntity("smokeeffect", {});
    particle_system->AllocatePool(particle_entity.id, 100, GibsUpdater);

    const mono::ITexturePtr texture = mono::GetTextureFactory()->CreateTexture("res/textures/particles/smoke_white_4.png");
    particle_system->SetPoolDrawData(particle_entity.id, texture, mono::BlendMode::SOURCE_ALPHA);

    m_particle_entity = particle_entity.id;
}

SmokeEffect::~SmokeEffect()
{
    m_particle_system->ReleasePool(m_particle_entity);
    g_entity_manager->ReleaseEntity(m_particle_entity);
}

void SmokeEffect::EmitSmokeAt(const math::Vector& position)
{
    m_particle_system->AttachEmitter(
        m_particle_entity, position, 0.5f, 20.0f, mono::EmitterType::BURST_REMOVE_ON_FINISH, SmokeGenerator);
}
