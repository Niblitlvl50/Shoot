
#include "TrailEffect.h"
#include "Particle/ParticleSystem.h"
#include "Rendering/RenderSystem.h"
#include "Rendering/Texture/ITextureFactory.h"
#include "Math/Matrix.h"
#include "TransformSystem/TransformSystem.h"

#include "EntitySystem/IEntityManager.h"
#include "Component.h"

using namespace game;

namespace
{
    void TrailGenerator(const math::Vector& position, mono::ParticlePoolComponentView& component_view)
    {
        constexpr float life = 0.5f;

        component_view.position = position;
        component_view.rotation = 0.0f;

        using namespace mono::Color;
        component_view.gradient = mono::Color::MakeGradient<4>(
            { 0.0f, 1.0f, 1.0f, 1.0f },
            { RED, RGBA(1.0f, 0.0f, 0.0f, 0.1f), RGBA(), RGBA() }
        );
        component_view.start_size = 16.0f;
        component_view.end_size = 10.0f;
        component_view.start_life = life;
        component_view.life = life;
    }
}

TrailEffect::TrailEffect(
    mono::TransformSystem* transform_system,
    mono::ParticleSystem* particle_system,
    mono::IEntityManager* entity_system,
    uint32_t follow_id)
    : m_particle_system(particle_system)
    , m_entity_system(entity_system)
{
    mono::Entity particle_entity = m_entity_system->CreateEntity("TrailEffect", { TRANSFORM_COMPONENT, PARTICLE_SYSTEM_COMPONENT });
    particle_system->SetPoolData(
        particle_entity.id,
        500,
        "res/textures/particles/flare.png",
        mono::BlendMode::ONE, 
        mono::ParticleTransformSpace::WORLD,
        0.0,
        mono::DefaultUpdater);

    const auto generator_proxy = [transform_system, follow_id](const math::Vector& position, mono::ParticlePoolComponentView& component_view) {
        const math::Matrix& world_transform = transform_system->GetWorld(follow_id);
        TrailGenerator(math::GetPosition(world_transform), component_view);
    };
    m_particle_system->AttachEmitter(particle_entity.id, math::ZeroVec, -1.0f, 100.0f, mono::EmitterType::CONTINOUS, generator_proxy);

    m_particle_entity = particle_entity.id;
}

TrailEffect::~TrailEffect()
{
    m_entity_system->ReleaseEntity(m_particle_entity);
}
