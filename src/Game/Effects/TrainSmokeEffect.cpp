
#include "TrainSmokeEffect.h"

#include "Particle/ParticleSystem.h"
#include "TransformSystem/TransformSystem.h"
#include "Util/Random.h"

#include "EntitySystem/IEntityManager.h"
#include "Entity/Component.h"

using namespace game;

namespace
{
    void TrainSmokeGenerator(const mono::ParticleGeneratorContext& context, mono::ParticlePoolComponentView& component_view)
    {
        const float x_variation = mono::Random(-0.02f, 0.02f);
        const float y_variation = mono::Random(-0.02f, 0.02f);
        const float x_velocity_variation = mono::Random(-0.1f, 0.1f);
        const float y_velocity_variation = mono::Random(0.3f, 0.6f);
        const float size = mono::Random(64.0f, 70.0f);
        const float end_size = mono::Random(80.0f, 120.0f);
        const float life = mono::Random(2.0f, 2.8f);

        component_view.position = context.position + math::Vector(x_variation, y_variation);
        component_view.rotation = 0.0f;
        component_view.velocity = math::Vector(x_velocity_variation, y_velocity_variation);
        component_view.angular_velocity = 0.0f; //mono::Random(-1.1f, 1.1f);

        using namespace mono::Color;
        component_view.color = mono::Color::MakeWithAlpha(OFF_WHITE, 0.75f);
        component_view.gradient = mono::Color::MakeGradient<4>(
            { 0.0f, 1.0f, 1.0f, 1.0f },
            { component_view.color, RGBA(1.0f, 1.0f, 1.0f, 0.0f), RGBA(), RGBA() }
        );
        component_view.start_size = size;
        component_view.end_size = end_size;
        component_view.size = size;

        component_view.start_life = life;
        component_view.life = life;
    }
}

TrainSmokeEffect::TrainSmokeEffect(mono::ParticleSystem* particle_system, mono::IEntityManager* entity_system, mono::TransformSystem* transform_system, uint32_t parent_entity_id)
    : m_particle_system(particle_system)
    , m_entity_system(entity_system)
{
    mono::Entity particle_entity = m_entity_system->CreateEntity("TrainSmokeEffect", { TRANSFORM_COMPONENT, PARTICLE_SYSTEM_COMPONENT });
    particle_system->SetPoolData(particle_entity.id,
        20,
        "res/textures/particles/smoke_white_6.png",
        mono::BlendMode::SOURCE_ALPHA,
        mono::ParticleDrawLayer::POST_GAMEOBJECTS,
        mono::ParticleTransformSpace::WORLD,
        0.01f,
        mono::DefaultUpdater);

    m_particle_entity = particle_entity.id;

    //transform_system->SetTransform(m_particle_entity, math::CreateMatrixWithPosition(math::Vector(0.2f, 0.0f)));
    transform_system->ChildTransform(m_particle_entity, parent_entity_id);
}

TrainSmokeEffect::~TrainSmokeEffect()
{
    m_entity_system->ReleaseEntity(m_particle_entity);
}

void TrainSmokeEffect::Start()
{
    m_emitter = m_particle_system->AttachEmitter(
        m_particle_entity,
        math::ZeroVec,
        -1.0f,
        5.0f,
        mono::EmitterType::CONTINOUS,
        mono::EmitterMode::AUTO_ACTIVATED,
        TrainSmokeGenerator);
}

void TrainSmokeEffect::Stop()
{
    if(m_emitter)
    {
        m_particle_system->ReleaseEmitter(m_particle_entity, m_emitter);
        m_emitter = nullptr;
    }
}

void TrainSmokeEffect::UpdateEmitterSpeed(float emit_rate)
{
    if(m_emitter)
    {
        m_emitter->emit_rate = emit_rate;
    }
}
