
#include "HealEffect.h"

#include "Particle/ParticleSystem.h"
#include "Rendering/RenderSystem.h"
#include "EntitySystem/IEntityManager.h"
#include "Entity/Component.h"
#include "Math/MathFunctions.h"
#include "Util/Random.h"

using namespace game;

HealEffect::HealEffect(mono::ParticleSystem* particle_system, mono::IEntityManager* entity_manager)
    : m_particle_system(particle_system)
    , m_entity_manager(entity_manager)
{
    mono::Entity particle_entity = m_entity_manager->CreateEntity("HealEffect", { TRANSFORM_COMPONENT, PARTICLE_SYSTEM_COMPONENT });
    particle_system->SetPoolData(
        particle_entity.id,
        64,
        "res/textures/particles/glyph1.png",
        mono::BlendMode::ONE,
        mono::ParticleDrawLayer::POST_GAMEOBJECTS,
        mono::ParticleTransformSpace::LOCAL,
        0.3f,
        mono::DefaultUpdater);

    m_particle_entity = particle_entity.id;
}

HealEffect::~HealEffect()
{
    m_entity_manager->ReleaseEntity(m_particle_entity);
}

void HealEffect::EmitAt(const math::Vector& position)
{
    const auto particle_generator = [](const mono::ParticleGeneratorContext& context, mono::ParticlePoolComponentView& view) {

        const math::Vector offset = math::Vector(
            mono::Random(-0.3f, 0.3f),
            mono::Random(-0.3f, 0.3f)
        );

        // Rise upward with slight sideways drift.
        const float speed = mono::Random(0.8f, 2.0f);
        const float drift = mono::Random(-0.3f, 0.3f);
        const math::Vector velocity = math::Vector(drift, speed);

        view.position = context.position + offset;
        view.velocity = velocity;
        view.rotation = 0.0f;
        view.angular_velocity = mono::Random(-60.0f, 60.0f);

        view.gradient = mono::Color::MakeGradient<4>(
            { 0.0f, 0.3f, 0.7f, 1.0f },
            {
                mono::Color::RGBA(0.5f, 1.0f, 0.5f, 0.0f),
                mono::Color::RGBA(0.3f, 1.0f, 0.3f, 1.0f),
                mono::Color::RGBA(0.2f, 0.8f, 0.2f, 0.7f),
                mono::Color::RGBA(0.1f, 0.6f, 0.1f, 0.0f)
            }
        );
        view.color = view.gradient.color[0];

        view.life = mono::Random(0.4f, 0.8f);
        view.start_life = view.life;

        view.size = 24.0f;
        view.start_size = 24.0f;
        view.end_size = 8.0f;
    };

    m_particle_system->AttachEmitter(
        m_particle_entity,
        position,
        0.5f,
        8.0f,
        mono::EmitterType::BURST_REMOVE_ON_FINISH,
        mono::EmitterMode::AUTO_ACTIVATED,
        particle_generator);
}
