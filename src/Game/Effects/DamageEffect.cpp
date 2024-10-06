
#include "DamageEffect.h"

#include "Util/Random.h"

#include "Math/MathFunctions.h"
#include "Math/EasingFunctions.h"

#include "Rendering/RenderSystem.h"
#include "Particle/ParticleSystem.h"
#include "EntitySystem/IEntityManager.h"
#include "Entity/Component.h"

using namespace game;

namespace
{
    void GibsGenerator(const mono::ParticleGeneratorContext& context, mono::ParticlePoolComponentView& component_view)
    {
        const float x = mono::Random(-4.0f, 4.0f);
        const float y = mono::Random(-4.0f, 4.0f);
        const float life = mono::Random(0.05f, 0.10f);

        component_view.position = context.position;
        component_view.velocity = math::Vector(x, y);

        component_view.rotation = 0.0f;

        using namespace mono::Color;

        component_view.color = OFF_WHITE;
        component_view.gradient = MakeGradient<4>(
            { 0.0f, 0.8f, 1.0f, 1.0f },
            { component_view.color, MakeWithAlpha(OFF_WHITE, 0.5f), MakeWithAlpha(OFF_WHITE, 0.0f), RGBA() }
        );

        constexpr float size = 24.0f;

        component_view.size = size;
        component_view.start_size = size;
        component_view.end_size = size - 8.0f;

        component_view.start_life = 0.1f + life;
        component_view.life = 0.1f + life;
    }
}

DamageEffect::DamageEffect(mono::ParticleSystem* particle_system, mono::IEntityManager* entity_system)
    : m_particle_system(particle_system)
    , m_entity_system(entity_system)
{
    mono::Entity particle_entity = m_entity_system->CreateEntity("DamageEffect", { TRANSFORM_COMPONENT, PARTICLE_SYSTEM_COMPONENT });
    m_particle_system->SetPoolData(
        particle_entity.id,
        100,
        "res/textures/particles/white_square.png",
        mono::BlendMode::SOURCE_ALPHA,
        mono::ParticleDrawLayer::POST_GAMEOBJECTS,
        mono::ParticleTransformSpace::LOCAL,
        0.001f,
        mono::DefaultUpdater);

    m_particle_entity = particle_entity.id;
}

DamageEffect::~DamageEffect()
{
    m_entity_system->ReleaseEntity(m_particle_entity);
}

void DamageEffect::EmitAt(const math::Vector& world_position)
{
    EmitAtWithDirection(world_position, 0.0f);
}

void DamageEffect::EmitAtWithDirection(const math::Vector& world_position, float direction)
{
    m_particle_system->AttachEmitter(
        m_particle_entity,
        world_position,
        0.25f,
        30.0f,
        mono::EmitterType::BURST_REMOVE_ON_FINISH,
        mono::EmitterMode::AUTO_ACTIVATED,
        GibsGenerator);
}

