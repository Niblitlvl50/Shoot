
#include "WeaponModifierEffect.h"

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
    void TrailGenerator(
        const mono::ParticleGeneratorContext& context, mono::ParticlePoolComponentView& component_view, const math::Vector& world_position)
    {
        const float life = mono::Random(0.5f, 2.0f);
        const float radians = mono::Random(0.0f, math::PI() * 2.0f);
        const math::Vector offset = math::VectorFromAngle(radians) * mono::Random(0.0f, 0.1f);

        component_view.position = world_position + offset;
        component_view.velocity = math::Vector(0.0f, -0.75f);
        component_view.rotation = 0.0f;
        component_view.angular_velocity = 0.0f;
        
        using namespace mono::Color;
        component_view.gradient = mono::Color::MakeGradient<4>(
            { 0.0f, 0.2f, 0.5f, 1.0f },
            { mono::Color::MakeWithAlpha(WHITE, 0.0f), mono::Color::MakeWithAlpha(WHITE, 1.0f), mono::Color::MakeWithAlpha(WHITE, 0.5f), mono::Color::MakeWithAlpha(WHITE, 0.0f) }
        );
        component_view.size = 24.0f;
        component_view.start_size = 24.0f;
        component_view.end_size = 38.0f;

        component_view.life = life;
        component_view.start_life = life;
    }
}

WeaponModifierEffect::WeaponModifierEffect(
    mono::TransformSystem* transform_system,
    mono::ParticleSystem* particle_system,
    mono::IEntityManager* entity_system)
    : m_transform_system(transform_system)
    , m_particle_system(particle_system)
    , m_entity_system(entity_system)
{
    mono::Entity particle_entity = m_entity_system->CreateEntity("WeaponModifierEffect", { TRANSFORM_COMPONENT, PARTICLE_SYSTEM_COMPONENT });
    m_particle_system->SetPoolData(
        particle_entity.id,
        20,
        "res/textures/particles/glyph3.png",
        mono::BlendMode::SOURCE_ALPHA,
        mono::ParticleDrawLayer::PRE_GAMEOBJECTS,
        mono::ParticleTransformSpace::WORLD,
        0.025f,
        mono::DefaultUpdater);

    m_particle_entity = particle_entity.id;
}

WeaponModifierEffect::~WeaponModifierEffect()
{
    m_entity_system->ReleaseEntity(m_particle_entity);
}

void WeaponModifierEffect::AttachToEntityWithOffset(uint32_t entity_id, const math::Vector& offset)
{
    math::Matrix& transform = m_transform_system->GetTransform(m_particle_entity);
    math::Translate(transform, offset);

    m_transform_system->ChildTransform(m_particle_entity, entity_id);
}

void WeaponModifierEffect::EmitForDuration(float duration_s)
{
    const auto generator_proxy = [this](const mono::ParticleGeneratorContext& context, mono::ParticlePoolComponentView& component_view) {
        const math::Vector& world_position = m_transform_system->GetWorldPosition(m_particle_entity);
        TrailGenerator(context, component_view, world_position);
    };

    mono::ParticleEmitterComponent* emitter = m_particle_system->AttachEmitter(
        m_particle_entity,
        math::ZeroVec,
        duration_s,
        3.0f,
        mono::EmitterType::CONTINOUS,
        mono::EmitterMode::AUTO_ACTIVATED,
        generator_proxy);

    (void)emitter;
}
