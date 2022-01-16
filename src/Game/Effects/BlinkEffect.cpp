
#include "BlinkEffect.h"

#include "Particle/ParticleSystem.h"
#include "Rendering/RenderSystem.h"
#include "Rendering/Texture/ITextureFactory.h"
#include "Util/Random.h"

#include "Math/MathFunctions.h"
#include "Math/EasingFunctions.h"
#include "EntitySystem/IEntityManager.h"

#include "Entity/Component.h"

using namespace game;

namespace
{
    void StartBlinkGenerator(const math::Vector& position, mono::ParticlePoolComponentView& component_view)
    {
        const float direction_variation = mono::Random(0.0f, math::PI() * 2.0f);
        math::Vector velocity = math::VectorFromAngle(direction_variation);
        math::Normalize(velocity);

        const float life = mono::Random(0.1f, 0.15f);
        const float velocity_variation = mono::Random(10.0f, 16.0f);
        const float size = mono::Random(12.0f, 16.0f);

        component_view.position = position + velocity;
        component_view.rotation = 0.0f;
        component_view.velocity = -velocity * velocity_variation;
        component_view.gradient = mono::Color::MakeGradient<4>(
            { 0.0f, 1.0f, 1.0f, 1.0f },
            { mono::Color::RGBA(1.0f, 0.8f, 0.0f, 1.0f), mono::Color::RGBA(0.5f, 0.1f, 0.0f, 0.0f), mono::Color::RGBA(), mono::Color::RGBA() }
        );
        component_view.start_size = size;
        component_view.end_size = 4.0f;
        component_view.size = size;
        component_view.start_life = life;
        component_view.life = life;
    }

    void EndBlinkGenerator(const math::Vector& position, mono::ParticlePoolComponentView& component_view)
    {
        const float direction_variation = mono::Random(0.0f, math::PI() * 2.0f);
        math::Vector velocity = math::VectorFromAngle(direction_variation);
        math::Normalize(velocity);

        const float life = mono::Random(0.1f, 0.15f);
        const float velocity_variation = mono::Random(10.0f, 16.0f);
        const float size = mono::Random(12.0f, 16.0f);

        component_view.position = position;
        component_view.rotation = 0.0f;
        component_view.velocity = velocity * velocity_variation;
        component_view.gradient = mono::Color::MakeGradient<4>(
            { 0.0f, 1.0f, 1.0f, 1.0f },
            { mono::Color::RGBA(1.0f, 0.8f, 0.0f, 1.0f), mono::Color::RGBA(0.5f, 0.1f, 0.0f, 0.0f), mono::Color::RGBA(), mono::Color::RGBA() }
        );
        component_view.start_size = size;
        component_view.end_size = 4.0f;
        component_view.size = size;
        component_view.start_life = life;
        component_view.life = life;
    }

    void GibsUpdater(mono::ParticlePoolComponentView& component_view, float delta_s)
    {
        const float t = 1.0f - float(component_view.life) / float(component_view.start_life);

        component_view.velocity *= 0.90f;
        component_view.position += component_view.velocity * delta_s;
        component_view.color = mono::Color::ColorFromGradient(component_view.gradient, t);
    }
}

BlinkEffect::BlinkEffect(mono::ParticleSystem* particle_system, mono::IEntityManager* entity_system)
    : m_particle_system(particle_system)
    , m_entity_system(entity_system)
{
    mono::Entity particle_entity = m_entity_system->CreateEntity("BlinkEffect", { TRANSFORM_COMPONENT, PARTICLE_SYSTEM_COMPONENT });
    m_particle_system->SetPoolData(
        particle_entity.id,
        100,
        "res/textures/particles/flare.png",
        mono::BlendMode::ONE,
        mono::ParticleTransformSpace::LOCAL,
        0.0f,
        GibsUpdater);

    m_particle_entity = particle_entity.id;
}

BlinkEffect::~BlinkEffect()
{
    m_entity_system->ReleaseEntity(m_particle_entity);
}

void BlinkEffect::EmitBlinkAwayAt(const math::Vector& position)
{
    m_particle_system->AttachEmitter(
        m_particle_entity, position, 0.5f, 50.0f, mono::EmitterType::BURST_REMOVE_ON_FINISH, StartBlinkGenerator);
}

void BlinkEffect::EmitBlinkBackAt(const math::Vector& position)
{
    m_particle_system->AttachEmitter(
        m_particle_entity, position, 0.5f, 50.0f, mono::EmitterType::BURST_REMOVE_ON_FINISH, EndBlinkGenerator);
}
