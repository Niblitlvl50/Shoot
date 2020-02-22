
#include "TrailEffect.h"
#include "Particle/ParticleSystem.h"
#include "Rendering/Texture/ITextureFactory.h"
#include "Math/Matrix.h"
#include "TransformSystem/TransformSystem.h"

#include "Factories.h"
#include "Entity/IEntityManager.h"

using namespace game;

namespace
{
    void TrailGenerator(const math::Vector& position, mono::ParticlePoolComponent& pool, size_t index)
    {
        constexpr int life = 500;

        pool.position[index] = position;
        pool.rotation[index] = 0.0f;
        pool.start_color[index] = mono::Color::RGBA(1.0f, 0.0f, 0.0f, 1.0f);
        pool.end_color[index] = mono::Color::RGBA(1.0f, 0.0f, 0.0f, 0.1f);
        pool.start_size[index] = 16.0f;
        pool.end_size[index] = 10.0f;
        pool.start_life[index] = life;
        pool.life[index] = life;
    }
}

TrailEffect::TrailEffect(mono::TransformSystem* transform_system, mono::ParticleSystem* particle_system, uint32_t follow_id)
    : m_particle_system(particle_system)
{
    mono::Entity particle_entity = g_entity_manager->CreateEntity("traileffect", {});
    particle_system->AllocatePool(particle_entity.id, 500, mono::DefaultUpdater);

    const mono::ITexturePtr texture = mono::GetTextureFactory()->CreateTexture("res/textures/flare.png");
    particle_system->SetPoolDrawData(particle_entity.id, texture, mono::BlendMode::ONE);

    const auto generator_proxy = [transform_system, follow_id](const math::Vector& position, mono::ParticlePoolComponent& pool, size_t index) {
        const math::Matrix& world_transform = transform_system->GetWorld(follow_id);
        TrailGenerator(math::GetPosition(world_transform), pool, index);
    };

    m_particle_system->AttachEmitter(particle_entity.id, math::ZeroVec, -1.0f, 100.0f, mono::EmitterType::CONTINOUS, generator_proxy);

    m_particle_entity = particle_entity.id;
}

TrailEffect::~TrailEffect()
{
    m_particle_system->ReleasePool(m_particle_entity);
    g_entity_manager->ReleaseEntity(m_particle_entity);
}
