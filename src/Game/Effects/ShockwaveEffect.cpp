
#include "ShockwaveEffect.h"
#include "EntitySystem/IEntityManager.h"
#include "Particle/ParticleSystem.h"
#include "TransformSystem/TransformSystem.h"
#include "Math/Vector.h"

using namespace game;

ShockwaveEffect::ShockwaveEffect(
    mono::TransformSystem* transform_system, mono::ParticleSystem* particle_system, mono::IEntityManager* entity_system)
    : m_transform_system(transform_system)
    , m_particle_system(particle_system)
    , m_entity_system(entity_system)
    , m_emitter(nullptr)
{
    mono::Entity particle_entity = m_entity_system->CreateEntity("res/entities/effect_shockwave.entity");

    std::vector<mono::ParticleEmitterComponent*> emitters = particle_system->GetAttachedEmitters(particle_entity.id);
    if(!emitters.empty())
        m_emitter = emitters.front();

    m_particle_entity = particle_entity.id;
}

ShockwaveEffect::~ShockwaveEffect()
{
    m_entity_system->ReleaseEntity(m_particle_entity);
}

void ShockwaveEffect::EmittAt(const math::Vector& position)
{
    if(!m_emitter)
        return;

    const math::Matrix new_transform = math::CreateMatrixWithPosition(position);
    m_transform_system->SetTransform(m_particle_entity, new_transform);
    m_particle_system->RestartEmitter(m_emitter);
}

