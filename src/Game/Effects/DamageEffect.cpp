
#include "DamageEffect.h"

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
        const float x = mono::Random(-4.0f, 4.0f);
        const float y = mono::Random(-4.0f, 4.0f);
        const float life = mono::Random(0.0f, 250.0f);

        component_view.position = position;
        component_view.velocity = math::Vector(x, y);

        component_view.rotation = 0.0f;

        component_view.color = mono::Color::OFF_WHITE;
        component_view.gradient = mono::Color::MakeGradient<3>(
            { 0.0f, 1.0f, 1.0f},
            { component_view.color, mono::Color::RGBA(0.9f, 0.9f, 0.9f, 0.5f), mono::Color::RGBA() }
        );

        constexpr float size = 32.0f;

        component_view.size = size;
        component_view.start_size = size;
        component_view.end_size = size;

        component_view.start_life = 100 + life;
        component_view.life = 100 + life;
    }
}

DamageEffect::DamageEffect(mono::ParticleSystem* particle_system, mono::IEntityManager* entity_system)
    : m_particle_system(particle_system)
    , m_entity_system(entity_system)
{
    mono::Entity particle_entity = m_entity_system->CreateEntity("DamageEffect", {});
    particle_system->AllocatePool(particle_entity.id, 500, mono::DefaultUpdater);

    const mono::ITexturePtr texture = mono::GetTextureFactory()->CreateTexture("res/textures/particles/flare.png");
    particle_system->SetPoolDrawData(particle_entity.id, texture, mono::BlendMode::SOURCE_ALPHA);

    m_particle_entity = particle_entity.id;
}

DamageEffect::~DamageEffect()
{
    m_particle_system->ReleasePool(m_particle_entity);
    m_entity_system->ReleaseEntity(m_particle_entity);
}

void DamageEffect::EmitGibsAt(const math::Vector& position, float direction)
{
    m_particle_system->AttachEmitter(
        m_particle_entity, position, 0.25f, 30.0f, mono::EmitterType::BURST_REMOVE_ON_FINISH, GibsGenerator);
}
