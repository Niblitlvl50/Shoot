
#include "MuzzleFlash.h"

#include "Particle/ParticleSystem.h"
#include "Rendering/Texture/TextureFactory.h"
#include "Util/Random.h"

#include "Math/MathFunctions.h"
#include "Actions/EasingFunctions.h"
#include "Factories.h"
#include "Entity/IEntityManager.h"

using namespace game;

namespace
{
    void GibsGenerator(const math::Vector& position, mono::ParticlePoolComponent& pool, size_t index, float direction)
    {
        constexpr float ten_degrees = math::ToRadians(20.0f);

        const float direction_variation = mono::Random(-ten_degrees, ten_degrees);
        const math::Vector& velocity = math::VectorFromAngle(direction + direction_variation);

        const int life = mono::RandomInt(100, 150);
        const float velocity_variation = mono::Random(10.0f, 16.0f);
        const float size = mono::Random(2.0f, 6.0f);

        pool.position[index] = position;
        pool.rotation[index] = 0.0f;
        pool.velocity[index] = velocity * velocity_variation;
        pool.start_color[index] = mono::Color::RGBA(1.0f, 0.8f, 0.0f, 1.0f);
        pool.end_color[index] = mono::Color::RGBA(0.5f, 0.1f, 0.0f, 0.0f);
        pool.start_size[index] = size;
        pool.end_size[index] = size;
        pool.size[index] = size;
        pool.start_life[index] = life;
        pool.life[index] = life;
    }

    void GibsUpdater(mono::ParticlePoolComponent& pool, size_t count, uint32_t delta_ms)
    {
        const float float_delta = float(delta_ms) / 1000.0f;

        for(size_t index = 0; index < count; ++index)
        {
            const float t = 1.0f - float(pool.life[index]) / float(pool.start_life[index]);
            const float t2 = float(pool.start_life[index]) - float(pool.life[index]);
            const float duration = float(pool.start_life[index]); // / 1000.0f;

            pool.velocity[index] *= 0.90;
            pool.position[index] += pool.velocity[index] * float_delta;
            //pool.m_size[index] = pool.m_start_size[index];

            const float alpha1 = pool.start_color[index].alpha;
            const float alpha2 = pool.end_color[index].alpha;
            
            pool.color[index] = mono::Color::LerpRGB(pool.start_color[index], pool.end_color[index], t);
            pool.color[index].alpha = game::EaseInCubic(t2, duration, alpha1, alpha2 - alpha1);
        }
    }    
}

MuzzleFlash::MuzzleFlash(mono::ParticleSystem* particle_system)
    : m_particle_system(particle_system)
{
    mono::Entity particle_entity = g_entity_manager->CreateEntity("muzzleflash", {});
    particle_system->AllocatePool(particle_entity.id, 500, GibsUpdater);

    const mono::ITexturePtr texture = mono::CreateTexture("res/textures/flare.png");
    particle_system->SetPoolDrawData(particle_entity.id, texture, mono::BlendMode::ONE);

    m_particle_entity = particle_entity.id;
}

MuzzleFlash::~MuzzleFlash()
{
    m_particle_system->ReleasePool(m_particle_entity);
    g_entity_manager->ReleaseEntity(m_particle_entity);
}

void MuzzleFlash::EmittAt(const math::Vector& position, float direction)
{
    const auto generator_proxy = [direction](const math::Vector& position, mono::ParticlePoolComponent& pool, size_t index) {
        GibsGenerator(position, pool, index, direction);
    };

    m_particle_system->AttachEmitter(
        m_particle_entity, position, 0.5f, 30.0f, mono::EmitterType::BURST_REMOVE_ON_FINISH, generator_proxy);
}

