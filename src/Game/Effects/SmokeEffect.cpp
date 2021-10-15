
#include "SmokeEffect.h"

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
    void SmokeGenerator(const math::Vector& position, mono::ParticlePoolComponentView& component_view)
    {
        static bool go_left = false;
        go_left = !go_left;

        const math::Vector velocity = go_left ? math::Vector(-1.0f, 0.0f) : math::Vector(1.0f, 0.0f);

        const float x_variation = mono::Random(-0.2f, 0.2f);
        const float y_variation = mono::Random(-0.2f, 0.2f);
        const float velocity_variation = mono::Random(1.0f, 3.0f);
        const float size = mono::Random(48.0f, 64.0f);
        const float end_size = mono::Random(64.0f, 80.0f);
        const int life = mono::RandomInt(200, 400);

        component_view.position = position + math::Vector(x_variation, y_variation);
        component_view.rotation = 0.0f;
        component_view.velocity = velocity * velocity_variation;
        //component_view.angular_velocity = mono::Random(-1.1f, 1.1f);
        
        using namespace mono::Color;
        component_view.gradient = mono::Color::MakeGradient<4>(
            { 0.0f, 1.0f, 1.0f, 1.0f },
            { OFF_WHITE, RGBA(1.0f, 1.0f, 1.0f, 0.0f), RGBA(), RGBA() }
        );
        component_view.start_size = size;
        component_view.end_size = end_size;
        component_view.size = size;
        component_view.start_life = life;
        component_view.life = life;
    }

    void GibsUpdater(mono::ParticlePoolComponentView& component_view, float delta_s)
    {
        const float t = 1.0f - float(component_view.life) / float(component_view.start_life);

        component_view.velocity *= 0.90;
        component_view.position += component_view.velocity * delta_s;
        component_view.size = (1.0f - t) * component_view.start_size + t * component_view.end_size;
        component_view.color = mono::Color::ColorFromGradient(component_view.gradient, t);
    }

    void SmokePillarGenerator(const math::Vector& position, mono::ParticlePoolComponentView& component_view)
    {
        const float x_offset = mono::Random(-0.025f, 0.025f);
        const math::Vector velocity = math::Vector(x_offset, 0.125f);

        const float start_size = 32.0f;
        const float end_size = mono::Random(128.0f, 256.0f);
        const int life = mono::RandomInt(25000, 30000);

        component_view.position = position;
        component_view.rotation = 0.0f;
        component_view.velocity = velocity;
        component_view.angular_velocity = 0.0f;

        using namespace mono::Color;
        component_view.gradient = mono::Color::MakeGradient<4>(
            { 0.0f, 0.1f, 0.8f, 1.0f },
            { RGBA(1.0f, 0.0f, 1.0f, 0.0f), RGBA(1.0f, 0.0f, 1.0f, 0.5f), RGBA(0.0f, 0.0f, 0.0f, 0.8f), RGBA(0.0f, 0.0f, 0.0f, 0.0f) }
            //{ RED, GREEN, BLUE, BLACK }
        );

        component_view.size = start_size;
        component_view.start_size = start_size;
        component_view.end_size = end_size;

        component_view.life = life;
        component_view.start_life = life;
    }

    void SmokePillarUpdater(mono::ParticlePoolComponentView& component_view, float delta_s)
    {
        const float t = 1.0f - float(component_view.life) / float(component_view.start_life);

        component_view.velocity *= 0.9999f;
        component_view.position += component_view.velocity * delta_s;
        component_view.size = (1.0f - t) * component_view.start_size + t * component_view.end_size;
        component_view.color = mono::Color::ColorFromGradient(component_view.gradient, t);
        component_view.rotation += component_view.angular_velocity * delta_s;
    }
}

SmokeEffect::SmokeEffect(mono::ParticleSystem* particle_system, mono::IEntityManager* entity_system)
    : m_particle_system(particle_system)
    , m_entity_system(entity_system)
{
    mono::Entity particle_entity = m_entity_system->CreateEntity("smokeeffect", {});
    particle_system->AllocatePool(particle_entity.id, 100, GibsUpdater);

    const mono::ITexturePtr texture = mono::GetTextureFactory()->CreateTexture("res/textures/particles/smoke_white_4.png");
    particle_system->SetPoolDrawData(particle_entity.id, texture, mono::BlendMode::SOURCE_ALPHA);

    m_particle_entity = particle_entity.id;
}

SmokeEffect::~SmokeEffect()
{
    m_particle_system->ReleasePool(m_particle_entity);
    m_entity_system->ReleaseEntity(m_particle_entity);
}

void SmokeEffect::EmitSmokeAt(const math::Vector& position)
{
    m_particle_system->AttachEmitter(
        m_particle_entity, position, 0.5f, 20.0f, mono::EmitterType::BURST_REMOVE_ON_FINISH, SmokeGenerator);
}


SmokePillarEffect::SmokePillarEffect(mono::ParticleSystem* particle_system, mono::IEntityManager* entity_system, mono::BlendMode blend_mode)
    : m_particle_system(particle_system)
    , m_entity_system(entity_system)
{
    mono::Entity particle_entity = m_entity_system->CreateEntity("smoke_pillar_effect", {});
    particle_system->AllocatePool(particle_entity.id, 100, SmokePillarUpdater);

    const mono::ITexturePtr texture = mono::GetTextureFactory()->CreateTexture("res/textures/particles/white_square.png");
    particle_system->SetPoolDrawData(particle_entity.id, texture, blend_mode);

    m_particle_entity = particle_entity.id;
}

SmokePillarEffect::~SmokePillarEffect()
{
    m_particle_system->ReleasePool(m_particle_entity);
    m_entity_system->ReleaseEntity(m_particle_entity);
}

void SmokePillarEffect::EmitSmokeAt(const math::Vector& position)
{
    m_particle_system->AttachEmitter(
        m_particle_entity, position, -1.0f, 2.5f, mono::EmitterType::CONTINOUS, SmokePillarGenerator);
}
