
#include "MuzzleFlash.h"

#include "Particle/ParticleSystem.h"
#include "Rendering/RenderSystem.h"
#include "Rendering/Texture/ITextureFactory.h"
#include "Util/Random.h"

#include "Math/MathFunctions.h"
#include "Math/EasingFunctions.h"
#include "EntitySystem/IEntityManager.h"

using namespace game;

namespace
{
    void GibsGenerator(const math::Vector& position, mono::ParticlePoolComponentView& component_view, float direction)
    {
        constexpr float ten_degrees = math::ToRadians(20.0f);

        const float direction_variation = mono::Random(-ten_degrees, ten_degrees);
        const math::Vector& velocity = math::VectorFromAngle(direction + direction_variation);

        const float life = mono::Random(0.1f, 0.15f);
        const float velocity_variation = mono::Random(10.0f, 16.0f);
        const float size = mono::Random(20.0f, 60.0f);

        component_view.position = position;
        component_view.rotation = 0.0f;
        component_view.velocity = velocity * velocity_variation;

        component_view.gradient = mono::Color::MakeGradient<4>(
            { 0.0f, 1.0f, 1.0f, 1.0f },
            { mono::Color::RGBA(1.0f, 0.8f, 0.0f, 1.0f), mono::Color::RGBA(0.5f, 0.1f, 0.0f, 0.0f), mono::Color::RGBA(), mono::Color::RGBA() }
        );

        component_view.start_size = size;
        component_view.end_size = size;
        component_view.size = size;
        component_view.start_life = life;
        component_view.life = life;
    }
}

MuzzleFlash::MuzzleFlash(mono::ParticleSystem* particle_system, mono::IEntityManager* entity_system)
    : m_particle_system(particle_system)
    , m_entity_system(entity_system)
{
    mono::Entity particle_entity = m_entity_system->CreateEntity("muzzleflash", {});
    particle_system->AllocatePool(particle_entity.id, 500, mono::DefaultUpdater);

    const mono::ITexturePtr texture = mono::GetTextureFactory()->CreateTexture("res/textures/particles/flare.png");
    particle_system->SetPoolDrawData(particle_entity.id, texture, mono::BlendMode::ONE, mono::ParticleTransformSpace::LOCAL);

    m_particle_entity = particle_entity.id;
}

MuzzleFlash::~MuzzleFlash()
{
    m_particle_system->ReleasePool(m_particle_entity);
    m_entity_system->ReleaseEntity(m_particle_entity);
}

void MuzzleFlash::EmittAt(const math::Vector& position, float direction)
{
    const auto generator_proxy = [direction](const math::Vector& position, mono::ParticlePoolComponentView& component_view) {
        GibsGenerator(position, component_view, direction);
    };

    m_particle_system->AttachEmitter(
        m_particle_entity, position, 0.5f, 30.0f, mono::EmitterType::BURST_REMOVE_ON_FINISH, generator_proxy);
}

