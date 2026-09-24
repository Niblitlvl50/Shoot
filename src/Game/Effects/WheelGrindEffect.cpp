
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
        constexpr float spread = math::ToRadians(30.0f);
        const float direction_variation = mono::Random(0.0f, -spread);
        const math::Vector& velocity = math::VectorFromAngle(direction + direction_variation);

        const math::Vector direction_position_offset = math::VectorFromAngle(direction + math::PI_2()) * mono::Random(0.0f, 0.25f);

        const float life = mono::Random(0.08f, 0.15f);
        const float velocity_variation = mono::Random(3.0f, 8.0f);
        const float size = mono::Random(10.0f, 14.0f);

        component_view.position = context.position + velocity * direction_position_offset;
        component_view.velocity = velocity * velocity_variation;

        component_view.rotation = 0.0f;
        component_view.angular_velocity = 0.0f;

        using namespace mono::Color;
        component_view.color = RGBA(1.0f, 1.0f, 0.8f, 1.0f);
        component_view.gradient = MakeGradient<4>(
            { 0.0f, 0.4f, 1.0f, 1.0f },
            { component_view.color, RGBA(1.0f, 0.6f, 0.1f, 1.0f), RGBA(1.0f, 0.2f, 0.0f, 0.1f), RGBA() }
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
        64,
        "res/textures/particles/white_square.png",
        mono::BlendMode::SOURCE_ALPHA,
        mono::ParticleDrawLayer::PRE_GAMEOBJECTS,
        mono::ParticleTransformSpace::WORLD,
        0.05f,
        mono::DefaultUpdater);

    m_particle_entity = particle_entity.id;
}

WheelGrindEffect::~WheelGrindEffect()
{
    m_entity_system->ReleaseEntity(m_particle_entity);
}

void WheelGrindEffect::Start()
{
    if(m_emitter)
        return;

    const auto generator_proxy = [this](const mono::ParticleGeneratorContext& context, mono::ParticlePoolComponentView& component_view) {
        SparkGenerator(context, component_view, m_direction);
    };
    
    m_emitter = m_particle_system->AttachEmitter(
        m_particle_entity,
        math::ZeroVec,
        -1.0f,
        64.0f,
        mono::EmitterType::CONTINOUS,
        mono::EmitterMode::AUTO_ACTIVATED,
        generator_proxy);
}

void WheelGrindEffect::Stop()
{
    if(m_emitter)
    {
        m_particle_system->ReleaseEmitter(m_particle_entity, m_emitter);
        m_emitter = nullptr;
    }
}

void WheelGrindEffect::EmitAtWithDirection(const math::Vector& world_position, float direction)
{
    m_direction = direction;
}

void WheelGrindEffect::EmitAt(const math::Vector& world_position)
{

}
