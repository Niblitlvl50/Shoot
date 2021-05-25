
#include "BlinkEffect.h"

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
    void StartBlinkGenerator(const math::Vector& position, mono::ParticlePoolComponentView& component_view)
    {
        const float direction_variation = mono::Random(0.0f, math::PI() * 2.0f);
        math::Vector velocity = math::VectorFromAngle(direction_variation);
        math::Normalize(velocity);

        const int life = mono::RandomInt(100, 150);
        const float velocity_variation = mono::Random(10.0f, 16.0f);
        const float size = mono::Random(12.0f, 16.0f);

        component_view.position = position + velocity;
        component_view.rotation = 0.0f;
        component_view.velocity = -velocity * velocity_variation;
        component_view.start_color = mono::Color::RGBA(1.0f, 0.8f, 0.0f, 1.0f);
        component_view.end_color = mono::Color::RGBA(0.5f, 0.1f, 0.0f, 0.0f);
        component_view.start_size = size;
        component_view.end_size = 4.0f;
        component_view.size = size;
        component_view.start_life = life;
        component_view.life = life;
    }

    void EndBlinkGenerator(const math::Vector& position, mono::ParticlePoolComponentView& component_view)
    {
        const float direction_variation = mono::Random(0.0f, math::PI() * 2.0f);
        math::Vector velocity = math::VectorFromAngle(direction_variation);
        math::Normalize(velocity);

        const int life = mono::RandomInt(100, 150);
        const float velocity_variation = mono::Random(10.0f, 16.0f);
        const float size = mono::Random(12.0f, 16.0f);

        component_view.position = position;
        component_view.rotation = 0.0f;
        component_view.velocity = velocity * velocity_variation;
        component_view.start_color = mono::Color::RGBA(1.0f, 0.8f, 0.0f, 1.0f);
        component_view.end_color = mono::Color::RGBA(0.5f, 0.1f, 0.0f, 0.0f);
        component_view.start_size = size;
        component_view.end_size = 4.0f;
        component_view.size = size;
        component_view.start_life = life;
        component_view.life = life;
    }

    void GibsUpdater(mono::ParticlePoolComponent& pool, size_t count, uint32_t delta_ms)
    {
        const float float_delta = float(delta_ms) / 1000.0f;

        for(size_t index = 0; index < count; ++index)
        {
            const float t = 1.0f - float(pool.life[index]) / float(pool.start_life[index]);
            const float t2 = float(pool.start_life[index]) - float(pool.life[index]);
            const float duration = float(pool.start_life[index]); // / 1000.0f;

            pool.velocity[index] *= 0.90;
            pool.position[index] += pool.velocity[index] * float_delta;
            //pool.m_size[index] = pool.m_start_size[index];

            const float alpha1 = pool.start_color[index].alpha;
            const float alpha2 = pool.end_color[index].alpha;
            
            pool.color[index] = mono::Color::LerpRGB(pool.start_color[index], pool.end_color[index], t);
            pool.color[index].alpha = math::EaseInCubic(t2, duration, alpha1, alpha2 - alpha1);
        }
    }    
}

BlinkEffect::BlinkEffect(mono::ParticleSystem* particle_system, mono::IEntityManager* entity_system)
    : m_particle_system(particle_system)
    , m_entity_system(entity_system)
{
    mono::Entity particle_entity = m_entity_system->CreateEntity("blinkeffect", {});
    particle_system->AllocatePool(particle_entity.id, 100, GibsUpdater);

    const mono::ITexturePtr texture = mono::GetTextureFactory()->CreateTexture("res/textures/particles/flare.png");
    particle_system->SetPoolDrawData(particle_entity.id, texture, mono::BlendMode::ONE);

    m_particle_entity = particle_entity.id;
}

BlinkEffect::~BlinkEffect()
{
    m_particle_system->ReleasePool(m_particle_entity);
    m_entity_system->ReleaseEntity(m_particle_entity);
}

void BlinkEffect::EmitBlinkAwayAt(const math::Vector& position)
{
    m_particle_system->AttachEmitter(
        m_particle_entity, position, 0.5f, 50.0f, mono::EmitterType::BURST_REMOVE_ON_FINISH, StartBlinkGenerator);
}

void BlinkEffect::EmitBlinkBackAt(const math::Vector& position)
{
    m_particle_system->AttachEmitter(
        m_particle_entity, position, 0.5f, 50.0f, mono::EmitterType::BURST_REMOVE_ON_FINISH, EndBlinkGenerator);
}
