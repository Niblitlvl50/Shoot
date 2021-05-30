
#include "ExplosionEffect.h"

#include "Util/Random.h"

#include "Math/MathFunctions.h"
#include "Math/EasingFunctions.h"

#include "Rendering/RenderSystem.h"
#include "Particle/ParticleSystem.h"
#include "EntitySystem/IEntityManager.h"

using namespace game;

namespace
{
    void GibsGenerator(const math::Vector& position, mono::ParticlePoolComponentView& component_view)
    {
        constexpr float area = 1.0f;
        const float x = mono::Random(-area, area);
        const float y = mono::Random(-area, area);
        const float life = mono::Random(100.0f, 250.0f);

        component_view.position = position + math::Vector(x, y);
        component_view.rotation = 0.0f;

        component_view.gradient = mono::Color::MakeGradient<3>(
            { 0.0f, 1.0f, 1.0f },
            { mono::Color::RGBA(1.0f, 0.6f, 0.2f, 1.0f), mono::Color::RGBA(0.5f, 0.3f, 0.1f, 0.0f), mono::Color::RGBA() }
        );

        const float size = mono::Random(48.0f, 96.0f);
        component_view.size = size;
        component_view.start_size = size;
        component_view.end_size = size;

        component_view.start_life = 100 + life;
        component_view.life = 100 + life;
    }
}

ExplosionEffect::ExplosionEffect(mono::ParticleSystem* particle_system, mono::IEntityManager* entity_system)
    : m_particle_system(particle_system)
    , m_entity_system(entity_system)
{
    mono::Entity particle_entity = m_entity_system->CreateEntity("ExplosionEffect", {});
    particle_system->AllocatePool(particle_entity.id, 500, mono::DefaultUpdater);

    const mono::ITexturePtr texture = mono::GetTextureFactory()->CreateTexture("res/textures/particles/white_square.png");
    particle_system->SetPoolDrawData(particle_entity.id, texture, mono::BlendMode::ONE);

    m_particle_entity = particle_entity.id;
}

ExplosionEffect::~ExplosionEffect()
{
    m_particle_system->ReleasePool(m_particle_entity);
    m_entity_system->ReleaseEntity(m_particle_entity);
}

void ExplosionEffect::ExplodeAt(const math::Vector& position)
{
    m_particle_system->AttachEmitter(
        m_particle_entity, position, 1.0f, 40.0f, mono::EmitterType::BURST_REMOVE_ON_FINISH, GibsGenerator);
}
