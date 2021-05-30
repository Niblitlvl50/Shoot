
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
    void TrailGenerator(const math::Vector& position, mono::ParticlePoolComponentView& component_view)
    {
        constexpr int life = 300;

        const float radians = mono::Random(0.0f, math::PI() * 2.0f);
        const math::Vector offset = math::VectorFromAngle(radians) * 0.1f;

        component_view.position = position + offset;
        component_view.velocity = math::ZeroVec;
        component_view.rotation = 0.0f;
        component_view.angular_velocity = 0.0f;
        component_view.gradient = mono::Color::MakeGradient<3>(
            { 0.0f, 1.0f, 1.0f },
            { mono::Color::RGBA(0.5f, 0.5f, 0.5f, 1.0f), mono::Color::RGBA(0.5f, 0.5f, 0.5f, 0.1f), mono::Color::RGBA() }
        );
        component_view.size = 10.0f;
        component_view.start_size = 10.0f;
        component_view.end_size = 10.0f;
        component_view.start_life = life;
        component_view.life = life;
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
    const auto generator_proxy = [this, entity_id](const math::Vector& position, mono::ParticlePoolComponentView& component_view) {
        const math::Matrix& world_transform = m_transform_system->GetWorld(entity_id);
        TrailGenerator(math::GetPosition(world_transform), component_view);
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

