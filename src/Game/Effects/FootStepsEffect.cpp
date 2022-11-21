
#include "FootStepsEffect.h"

#include "Particle/ParticleSystem.h"
#include "Rendering/RenderSystem.h"
#include "Rendering/Texture/ITextureFactory.h"
#include "Math/Matrix.h"
#include "Math/MathFunctions.h"
#include "TransformSystem/TransformSystem.h"
#include "Util/Random.h"

#include "EntitySystem/IEntityManager.h"
#include "Entity/Component.h"

using namespace game;

namespace
{
    void TrailGenerator(const math::Vector& position, mono::ParticlePoolComponentView& component_view)
    {
        constexpr float life = 0.25f;

        const float radians = mono::Random(0.0f, math::PI() * 2.0f);
        const math::Vector offset = math::VectorFromAngle(radians) * 0.05f;

        component_view.position = position + offset;
        component_view.velocity = math::ZeroVec;
        component_view.rotation = 0.0f;
        component_view.angular_velocity = 0.0f;
        
        using namespace mono::Color;
        component_view.gradient = mono::Color::MakeGradient<4>(
            { 0.0f, 1.0f, 1.0f, 1.0f },
            { RGBA(0.5f, 0.5f, 0.5f, 0.1f), RGBA(0.5f, 0.5f, 0.5f, 0.0f), RGBA(), RGBA() }
        );
        component_view.size = 20.0f;
        component_view.start_size = 20.0f;
        component_view.end_size = 30.0f;
        component_view.start_life = life;
        component_view.life = life;
    }
}

FootStepsEffect::FootStepsEffect(mono::ParticleSystem* particle_system, mono::IEntityManager* entity_system)
    : m_particle_system(particle_system)
    , m_entity_system(entity_system)
{
    mono::Entity particle_entity = m_entity_system->CreateEntity("FootStepsEffect", { TRANSFORM_COMPONENT, PARTICLE_SYSTEM_COMPONENT });
    m_particle_system->SetPoolData(
        particle_entity.id,
        50,
        "res/textures/particles/white_square.png",
        mono::BlendMode::ONE,
        mono::ParticleDrawLayer::PRE_GAMEOBJECTS,
        mono::ParticleTransformSpace::LOCAL,
        0.0f,
        mono::DefaultUpdater);

    m_particle_entity = particle_entity.id;
}

FootStepsEffect::~FootStepsEffect()
{
    m_entity_system->ReleaseEntity(m_particle_entity);
}

void FootStepsEffect::EmitFootStepsAt(const math::Vector& world_position)
{
    m_particle_system->AttachEmitter(
        m_particle_entity,
        world_position,
        1.0f,
        10.0f,
        mono::EmitterType::BURST_REMOVE_ON_FINISH,
        TrailGenerator);
}
