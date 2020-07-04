
#include "DamageEffect.h"

#include "Util/Random.h"

#include "Math/MathFunctions.h"
#include "Actions/EasingFunctions.h"

#include "Particle/ParticleSystem.h"
#include "Factories.h"
#include "Entity/IEntityManager.h"

using namespace game;

namespace
{
    void GibsGenerator(const math::Vector& position, mono::ParticlePoolComponent& pool, size_t index)
    {
        const float x = mono::Random(-4.0f, 4.0f);
        const float y = mono::Random(-4.0f, 4.0f);
        const float life = mono::Random(0.0f, 250.0f);

        pool.position[index] = position;
        pool.velocity[index] = math::Vector(x, y);
        pool.rotation[index] = 0.0f;
        //pool.m_color[index];
        pool.start_color[index] = mono::Color::RGBA(1.0f, 0.0f, 0.0f, 1.0f);
        pool.end_color[index] = mono::Color::RGBA(0.0f, 1.0f, 0.0f, 0.1f);

        pool.size[index] = 10.0f;
        pool.start_size[index] = 15.0f;
        pool.end_size[index] = 10.0f;

        pool.start_life[index] = 100 + life;
        pool.life[index] = 100 + life;
    }

/*
    void DamageParticleUpdater(mono::ParticlePoolComponent& pool, size_t count, uint32_t delta_ms)
    {
        const float float_delta = float(delta) / 1000.0f;

        for(size_t index = 0; index < count; ++index)
        {
            const float t = 1.0f - float(pool.m_life[index]) / float(pool.m_start_life[index]);
            const float t2 = float(pool.m_start_life[index]) - float(pool.m_life[index]);
            const float duration = float(pool.m_start_life[index]); // / 1000.0f;

            pool.m_velocity[index] *= 0.90;
            pool.m_position[index] += pool.m_velocity[index] * float_delta;
            //pool.m_size[index] = pool.m_start_size[index];

            const float alpha1 = pool.m_start_color[index].alpha;
            const float alpha2 = pool.m_end_color[index].alpha;
            
            pool.m_color[index] = mono::Color::LerpRGB(pool.m_start_color[index], pool.m_end_color[index], t);
            pool.m_color[index].alpha = game::EaseInCubic(t2, duration, alpha1, alpha2 - alpha1);
        }
    }
*/
}

DamageEffect::DamageEffect(mono::ParticleSystem* particle_system)
    : m_particle_system(particle_system)
{
    mono::Entity particle_entity = g_entity_manager->CreateEntity("DamageEffect", {});
    particle_system->AllocatePool(particle_entity.id, 500, mono::DefaultUpdater);

    const mono::ITexturePtr texture = mono::GetTextureFactory()->CreateTexture("res/textures/flare.png");
    particle_system->SetPoolDrawData(particle_entity.id, texture, mono::BlendMode::ONE);

    m_particle_entity = particle_entity.id;
}

DamageEffect::~DamageEffect()
{
    m_particle_system->ReleasePool(m_particle_entity);
    g_entity_manager->ReleaseEntity(m_particle_entity);
}

void DamageEffect::EmitGibsAt(const math::Vector& position, float direction)
{
    m_particle_system->AttachEmitter(
        m_particle_entity, position, 0.25f, 30.0f, mono::EmitterType::BURST_REMOVE_ON_FINISH, GibsGenerator);
}

void DamageEffect::EmitBloodAt(const math::Vector& position, float direction)
{

}

