
#include "PickupEffect.h"

#include "Particle/ParticleSystem.h"
#include "Rendering/RenderSystem.h"
#include "Rendering/Texture/ITextureFactory.h"
#include "Util/Random.h"

#include "Math/MathFunctions.h"
#include "Math/EasingFunctions.h"
#include "EntitySystem/IEntityManager.h"
#include "Entity/Component.h"

using namespace game;

PickupEffect::PickupEffect(mono::ParticleSystem* particle_system, mono::IEntityManager* entity_system)
    : m_particle_system(particle_system)
    , m_entity_system(entity_system)
{
    mono::Entity particle_entity = m_entity_system->CreateEntity("PickupEffect", { TRANSFORM_COMPONENT, PARTICLE_SYSTEM_COMPONENT });
    particle_system->SetPoolData(
        particle_entity.id,
        20,
        "res/textures/particles/glyph1.png",
        mono::BlendMode::ONE,
        mono::ParticleDrawLayer::POST_GAMEOBJECTS,
        mono::ParticleTransformSpace::LOCAL,
        0.2f,
        mono::DefaultUpdater);

    m_particle_entity = particle_entity.id;
}

PickupEffect::~PickupEffect()
{
    m_entity_system->ReleaseEntity(m_particle_entity);
}

void PickupEffect::EmitAt(const math::Vector& position)
{
    const auto particle_generator = [](const mono::ParticleGeneratorContext& context, mono::ParticlePoolComponentView& view) {

        const math::Vector offset = math::Vector(
            mono::Random(-0.1f, 0.1f),
            mono::Random(-0.1f, 0.1f)
        );

        const float magnitude_variation = mono::Random(2.0f, 3.0f);
        const float direction = mono::Random(0.0f, 360.0f);
        const math::Vector& velocity = math::VectorFromAngle(math::ToRadians(direction)) * magnitude_variation;

        view.position = context.position + offset;
        view.velocity = velocity;
        view.rotation = 0.0f;
        view.angular_velocity = 0.0f;

        view.gradient = mono::Color::MakeGradient<4>(
            { 0.0f, 0.25f, 0.5f, 1.0f },
            {
                mono::Color::RGBA(1.0f, 1.0f, 1.0f, 0.0f),
                mono::Color::RGBA(0.9f, 0.1f, 0.1f, 1.0f),
                mono::Color::RGBA(0.8f, 0.0f, 0.8f, 1.0f),
                mono::Color::RGBA(0.6f, 0.0f, 1.0f, 0.0f)
            }
        );
        view.color = view.gradient.color[0];

        view.life = mono::Random(0.2f, 0.5f);
        view.start_life = view.life;

        view.size = 32.0f;
        view.start_size = 32.0f;
        view.end_size = 32.0f;
    };

    m_particle_system->AttachEmitter(
        m_particle_entity, position, 1.0f, 20.0f, mono::EmitterType::BURST_REMOVE_ON_FINISH, particle_generator);
}

