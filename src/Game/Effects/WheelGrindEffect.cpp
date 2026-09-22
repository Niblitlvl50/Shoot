
#include "WheelGrindEffect.h"

#include "Particle/ParticleSystem.h"
#include "Util/Random.h"

#include "Math/MathFunctions.h"
#include "EntitySystem/IEntityManager.h"
#include "Entity/Component.h"

using namespace game;

namespace
{
    void SparkGenerator(const mono::ParticleGeneratorContext& context, mono::ParticlePoolComponentView& component_view, float direction)
    {
        constexpr float spread = math::ToRadians(50.0f);
        const float direction_variation = mono::Random(-spread, spread);
        const math::Vector& velocity = math::VectorFromAngle(direction + direction_variation);

        const float life = mono::Random(0.08f, 0.2f);
        const float velocity_variation = mono::Random(3.0f, 8.0f);
        const float size = mono::Random(4.0f, 8.0f);

        component_view.position = context.position;
        component_view.velocity = velocity * velocity_variation;

        component_view.rotation = 0.0f;
        component_view.angular_velocity = 0.0f;

        using namespace mono::Color;
        component_view.gradient = MakeGradient<4>(
            { 0.0f, 0.4f, 1.0f, 1.0f },
            { RGBA(1.0f, 1.0f, 0.8f, 1.0f), RGBA(1.0f, 0.6f, 0.1f, 1.0f), RGBA(1.0f, 0.2f, 0.0f, 0.5f), RGBA() }
        );

        component_view.size = size;
        component_view.start_size = size;
        component_view.end_size = size * 0.5f;

        component_view.start_life = life;
        component_view.life = life;
    }
}

WheelGrindEffect::WheelGrindEffect(mono::ParticleSystem* particle_system, mono::IEntityManager* entity_system)
    : m_particle_system(particle_system)
    , m_entity_system(entity_system)
{
    mono::Entity particle_entity = m_entity_system->CreateEntity("WheelGrindEffect", { TRANSFORM_COMPONENT, PARTICLE_SYSTEM_COMPONENT });
    m_particle_system->SetPoolData(
        particle_entity.id,
        40,
        "res/textures/particles/white_square.png",
        mono::BlendMode::SOURCE_ALPHA,
        mono::ParticleDrawLayer::POST_GAMEOBJECTS,
        mono::ParticleTransformSpace::LOCAL,
        0.0f,
        mono::DefaultUpdater);

    m_particle_entity = particle_entity.id;
}

WheelGrindEffect::~WheelGrindEffect()
{
    m_entity_system->ReleaseEntity(m_particle_entity);
}

void WheelGrindEffect::EmitAtWithDirection(const math::Vector& world_position, float direction)
{
    const auto generator_proxy = [direction](const mono::ParticleGeneratorContext& context, mono::ParticlePoolComponentView& component_view) {
        SparkGenerator(context, component_view, direction);
    };

    m_particle_system->AttachEmitter(
        m_particle_entity,
        world_position,
        0.1f,
        60.0f,
        mono::EmitterType::BURST_REMOVE_ON_FINISH,
        mono::EmitterMode::AUTO_ACTIVATED,
        generator_proxy);
}

void WheelGrindEffect::EmitAt(const math::Vector& world_position)
{

}
