
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
        const float size = mono::Random(2.0f, 6.0f);

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

    void GibsUpdater(mono::ParticlePoolComponentView& component_view, float delta_s)
    {
        const float t = 1.0f - float(component_view.life) / float(component_view.start_life);
        //const float t2 = float(pool.start_life[index]) - float(pool.life[index]);
        //const float duration = float(pool.start_life[index]); // / 1000.0f;

        component_view.velocity *= 0.90;
        component_view.position += component_view.velocity * delta_s;
        //pool.m_size[index] = pool.m_start_size[index];

        component_view.color = mono::Color::ColorFromGradient(component_view.gradient, t);

        /*
        const float alpha1 = pool.start_color[index].alpha;
        const float alpha2 = pool.end_color[index].alpha;
        pool.color[index] = mono::Color::LerpRGB(pool.start_color[index], pool.end_color[index], t);
        pool.color[index].alpha = math::EaseInCubic(t2, duration, alpha1, alpha2 - alpha1);
        */
    }
}

MuzzleFlash::MuzzleFlash(mono::ParticleSystem* particle_system, mono::IEntityManager* entity_system)
    : m_particle_system(particle_system)
    , m_entity_system(entity_system)
{
    mono::Entity particle_entity = m_entity_system->CreateEntity("muzzleflash", {});
    particle_system->AllocatePool(particle_entity.id, 500, GibsUpdater);

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

