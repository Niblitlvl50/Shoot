
#include "CargoTransferEffect.h"

#include "Particle/ParticleSystem.h"
#include "Rendering/RenderSystem.h"
#include "Util/Random.h"

#include "Math/MathFunctions.h"
#include "EntitySystem/IEntityManager.h"
#include "Entity/Component.h"

using namespace game;

CargoTransferEffect::CargoTransferEffect(mono::ParticleSystem* particle_system, mono::IEntityManager* entity_system)
    : m_particle_system(particle_system)
    , m_entity_system(entity_system)
{
    mono::Entity particle_entity = m_entity_system->CreateEntity("CargoTransferEffect", { TRANSFORM_COMPONENT, PARTICLE_SYSTEM_COMPONENT });
    particle_system->SetPoolData(
        particle_entity.id,
        100,
        "res/textures/particles/smoke_white_6.png",
        mono::BlendMode::SOURCE_ALPHA,
        mono::ParticleDrawLayer::POST_GAMEOBJECTS,
        mono::ParticleTransformSpace::LOCAL,
        0.01f,
        mono::DefaultUpdater);

    m_particle_entity = particle_entity.id;
}

CargoTransferEffect::~CargoTransferEffect()
{
    m_entity_system->ReleaseEntity(m_particle_entity);
}

void CargoTransferEffect::EmitAt(const math::Vector& position)
{
    const auto particle_generator = [](const mono::ParticleGeneratorContext& context, mono::ParticlePoolComponentView& view) {

        const math::Vector offset = math::Vector(
            mono::Random(-0.4f, 0.4f),
            mono::Random(-0.3f, 0.1f)
        );

        // Mostly upwards, spreading out a little to the sides.
        const float direction = mono::Random(-30.0f, 30.0f);
        const float magnitude = mono::Random(0.5f, 1.5f);

        view.position = context.position + offset;
        view.velocity = math::VectorFromAngle(math::ToRadians(direction)) * magnitude;
        view.rotation = math::ToRadians(mono::Random(0.0f, 360.0f));
        view.angular_velocity = math::ToRadians(mono::Random(-60.0f, 60.0f));

        view.gradient = mono::Color::MakeGradient<4>(
            { 0.0f, 0.2f, 0.6f, 1.0f },
            {
                mono::Color::RGBA(1.0f, 0.95f, 0.8f, 0.0f),
                mono::Color::RGBA(0.95f, 0.85f, 0.6f, 0.8f),
                mono::Color::RGBA(0.8f, 0.7f, 0.5f, 0.5f),
                mono::Color::RGBA(0.6f, 0.55f, 0.45f, 0.0f)
            }
        );
        view.color = view.gradient.color[0];

        view.life = mono::Random(0.5f, 0.9f);
        view.start_life = view.life;

        view.start_size = mono::Random(24.0f, 40.0f);
        view.end_size = view.start_size * 2.0f;
        view.size = view.start_size;
    };

    m_particle_system->AttachEmitter(
        m_particle_entity,
        position,
        0.1f,
        150.0f,
        mono::EmitterType::BURST_REMOVE_ON_FINISH,
        mono::EmitterMode::AUTO_ACTIVATED,
        particle_generator);
}
