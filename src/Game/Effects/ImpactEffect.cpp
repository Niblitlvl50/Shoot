
#include "ImpactEffect.h"

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
    void GibsGenerator(const mono::ParticleGeneratorContext& context, mono::ParticlePoolComponentView& component_view, float direction)
    {
        constexpr float ten_degrees = math::ToRadians(20.0f);

        const float direction_variation = mono::Random(-ten_degrees, ten_degrees);
        const math::Vector& velocity = math::VectorFromAngle(direction + direction_variation);

        const float life = mono::Random(0.1f, 0.125f);
        const float velocity_variation = mono::Random(5.0f, 10.0f);
        const float size = mono::Random(12.0f, 16.0f);

        component_view.position = context.position;
        component_view.velocity = velocity * velocity_variation;

        component_view.rotation = 0.0f;
        component_view.angular_velocity = 0.0f;

        using namespace mono::Color;

        component_view.color = ORANGE;
        component_view.gradient = MakeGradient<4>(
            { 0.0f, 1.0f, 1.0f, 1.0f },
            { ORANGE, RGBA(1.0f, 1.0f, 1.0f, 0.0f), RGBA(), RGBA() }
        );

        component_view.size = size;
        component_view.start_size = size;
        component_view.end_size = size;

        component_view.start_life = life;
        component_view.life = life;
    }
}

ImpactEffect::ImpactEffect(mono::ParticleSystem* particle_system, mono::IEntityManager* entity_system)
    : m_particle_system(particle_system)
    , m_entity_system(entity_system)
{
    mono::Entity particle_entity = m_entity_system->CreateEntity("ImpactEffect", { TRANSFORM_COMPONENT, PARTICLE_SYSTEM_COMPONENT });
    m_particle_system->SetPoolData(
        particle_entity.id,
        50,
        "res/textures/particles/white_square.png",
        mono::BlendMode::SOURCE_ALPHA,
        mono::ParticleDrawLayer::POST_GAMEOBJECTS,
        mono::ParticleTransformSpace::LOCAL,
        0.0f,
        mono::DefaultUpdater);

    m_particle_entity = particle_entity.id;
}

ImpactEffect::~ImpactEffect()
{
    m_entity_system->ReleaseEntity(m_particle_entity);
}

void ImpactEffect::EmitAtWithDirection(const math::Vector& world_position, float direction)
{
    const auto generator_proxy = [direction](const mono::ParticleGeneratorContext& context, mono::ParticlePoolComponentView& component_view) {
        GibsGenerator(context, component_view, direction);
    };

    m_particle_system->AttachEmitter(
        m_particle_entity,
        world_position,
        0.5f,
        30.0f,
        mono::EmitterType::BURST_REMOVE_ON_FINISH,
        mono::EmitterMode::AUTO_ACTIVATED,
        generator_proxy);
}

void ImpactEffect::EmitAt(const math::Vector& world_position)
{

}
