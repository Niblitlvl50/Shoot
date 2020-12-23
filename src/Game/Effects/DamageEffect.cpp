
#include "DamageEffect.h"

#include "Util/Random.h"

#include "Math/MathFunctions.h"
#include "Math/EasingFunctions.h"

#include "Particle/ParticleSystem.h"
#include "Factories.h"
#include "EntitySystem/IEntityManager.h"

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
        pool.color[index] = mono::Color::RGBA(1.0f, 0.0f, 0.0f, 1.0f);
        pool.start_color[index] = pool.color[index];
        pool.end_color[index] = mono::Color::RGBA(0.0f, 0.0f, 0.0f, 0.5f);

        pool.size[index] = 16.0f;
        pool.start_size[index] = 16.0f;
        pool.end_size[index] = 16.0f;

        pool.start_life[index] = 100 + life;
        pool.life[index] = 100 + life;
    }
}

DamageEffect::DamageEffect(mono::ParticleSystem* particle_system)
    : m_particle_system(particle_system)
{
    mono::Entity particle_entity = g_entity_manager->CreateEntity("DamageEffect", {});
    particle_system->AllocatePool(particle_entity.id, 500, mono::DefaultUpdater);

    const mono::ITexturePtr texture = mono::GetTextureFactory()->CreateTexture("res/textures/particles/flare.png");
    particle_system->SetPoolDrawData(particle_entity.id, texture, mono::BlendMode::SOURCE_ALPHA);

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
