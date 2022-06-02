
#include "SmokeEffect.h"

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
    void SmokeGenerator(const math::Vector& position, mono::ParticlePoolComponentView& component_view)
    {
        static bool go_left = false;
        go_left = !go_left;

        const math::Vector velocity = go_left ? math::Vector(-1.0f, 0.0f) : math::Vector(1.0f, 0.0f);

        const float x_variation = mono::Random(-0.2f, 0.2f);
        const float y_variation = mono::Random(-0.2f, 0.2f);
        const float velocity_variation = mono::Random(1.0f, 3.0f);
        const float size = mono::Random(64.0f, 80.0f);
        const float end_size = mono::Random(80.0f, 100.0f);
        const float life = mono::Random(0.4f, 0.8f);

        component_view.position = position + math::Vector(x_variation, y_variation);
        component_view.rotation = 0.0f;
        component_view.velocity = velocity * velocity_variation;
        //component_view.angular_velocity = mono::Random(-1.1f, 1.1f);
        
        using namespace mono::Color;
        component_view.gradient = mono::Color::MakeGradient<4>(
            { 0.0f, 1.0f, 1.0f, 1.0f },
            { OFF_WHITE, RGBA(1.0f, 1.0f, 1.0f, 0.0f), RGBA(), RGBA() }
        );
        component_view.start_size = size;
        component_view.end_size = end_size;
        component_view.size = size;
        component_view.start_life = life;
        component_view.life = life;
    }

    void GibsUpdater(mono::ParticlePoolComponentView& component_view, float delta_s)
    {
        const float t = 1.0f - float(component_view.life) / float(component_view.start_life);

        component_view.velocity *= 0.90f;
        component_view.position += component_view.velocity * delta_s;
        component_view.size = (1.0f - t) * component_view.start_size + t * component_view.end_size;
        component_view.color = mono::Color::ColorFromGradient(component_view.gradient, t);
    }
}

SmokeEffect::SmokeEffect(mono::ParticleSystem* particle_system, mono::IEntityManager* entity_system)
    : m_particle_system(particle_system)
    , m_entity_system(entity_system)
{
    mono::Entity particle_entity = m_entity_system->CreateEntity("SmokeEffect", { TRANSFORM_COMPONENT, PARTICLE_SYSTEM_COMPONENT });
    particle_system->SetPoolData(particle_entity.id,
        100,
        "res/textures/particles/smoke_white_4.png",
        mono::BlendMode::SOURCE_ALPHA,
        mono::ParticleDrawLayer::PRE_GAMEOBJECTS,
        mono::ParticleTransformSpace::LOCAL,
        0.0f,
        GibsUpdater);

    m_particle_entity = particle_entity.id;
}

SmokeEffect::~SmokeEffect()
{
    m_entity_system->ReleaseEntity(m_particle_entity);
}

void SmokeEffect::EmitSmokeAt(const math::Vector& position)
{
    m_particle_system->AttachEmitter(
        m_particle_entity, position, 0.5f, 20.0f, mono::EmitterType::BURST_REMOVE_ON_FINISH, SmokeGenerator);
}
