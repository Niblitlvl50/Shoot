
#include "BulletTrailEffect.h"

#include "Particle/ParticleSystem.h"
#include "Rendering/RenderSystem.h"
#include "Rendering/Texture/ITextureFactory.h"
#include "Math/Matrix.h"
#include "Math/MathFunctions.h"
#include "TransformSystem/TransformSystem.h"
#include "Util/Random.h"

#include "EntitySystem/IEntityManager.h"

using namespace game;

namespace
{
    void TrailGenerator(const math::Vector& position, mono::ParticlePoolComponent& pool, size_t index)
    {
        constexpr int life = 300;

        const float radians = mono::Random(0.0f, math::PI() * 2.0f);
        const math::Vector offset = math::VectorFromAngle(radians) * 0.1f;

        pool.position[index] = position + offset;
        pool.velocity[index] = math::ZeroVec;
        pool.rotation[index] = 0.0f;
        pool.angular_velocity[index] = 0.0f;
        pool.color[index] = mono::Color::RGBA(0.5f, 0.5f, 0.5f, 1.0f);
        pool.start_color[index] = mono::Color::RGBA(0.5f, 0.5f, 0.5f, 1.0f);
        pool.end_color[index] = mono::Color::RGBA(0.5f, 0.5f, 0.5f, 0.1f);
        pool.size[index] = 10.0f;
        pool.start_size[index] = 10.0f;
        pool.end_size[index] = 10.0f;
        pool.start_life[index] = life;
        pool.life[index] = life;
    }
}

BulletTrailEffect::BulletTrailEffect(
    mono::TransformSystem* transform_system,
    mono::ParticleSystem* particle_system,
    mono::IEntityManager* entity_system)
    : m_transform_system(transform_system)
    , m_particle_system(particle_system)
    , m_entity_system(entity_system)
{
    mono::Entity particle_entity = m_entity_system->CreateEntity("bullettraileffect", {});
    particle_system->AllocatePool(particle_entity.id, 500, mono::DefaultUpdater);

    const mono::ITexturePtr texture = mono::GetTextureFactory()->CreateTexture("res/textures/particles/white_square.png");
    particle_system->SetPoolDrawData(particle_entity.id, texture, mono::BlendMode::ONE);

    m_particle_entity = particle_entity.id;
}

BulletTrailEffect::~BulletTrailEffect()
{
    m_particle_system->ReleasePool(m_particle_entity);
    m_entity_system->ReleaseEntity(m_particle_entity);
}

void BulletTrailEffect::AttachEmitterToBullet(uint32_t entity_id)
{
    const auto generator_proxy = [this, entity_id](const math::Vector& position, mono::ParticlePoolComponent& pool, size_t index) {
        const math::Matrix& world_transform = m_transform_system->GetWorld(entity_id);
        TrailGenerator(math::GetPosition(world_transform), pool, index);
    };

    mono::ParticleEmitterComponent* emitter = m_particle_system->AttachEmitter(
        m_particle_entity,
        math::ZeroVec,
        -1.0f,
        50.0f,
        mono::EmitterType::CONTINOUS,
        generator_proxy);

    m_bullet_id_to_emitter[entity_id] = emitter;
}

void BulletTrailEffect::RemoveEmitterFromBullet(uint32_t entity_id)
{
    const auto it = m_bullet_id_to_emitter.find(entity_id);
    if(it != m_bullet_id_to_emitter.end())
    {
        m_particle_system->ReleaseEmitter(m_particle_entity, it->second);
        m_bullet_id_to_emitter.erase(entity_id);
    }
}

