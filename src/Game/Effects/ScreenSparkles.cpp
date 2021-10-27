
#include "ScreenSparkles.h"
#include "Particle/ParticleSystem.h"
#include "Rendering/RenderSystem.h"
#include "Rendering/Texture/ITextureFactory.h"
#include "Util/Random.h"

#include "Math/MathFunctions.h"
#include "Math/Quad.h"
#include "Factories.h"
#include "EntitySystem/IEntityManager.h"

using namespace game;

namespace
{
    void Generator(const math::Vector& position, mono::ParticlePoolComponentView& component_view, const math::Vector& viewport_size)
    {
        const float half_width = viewport_size.x / 2.0f;
        const float half_height = viewport_size.y / 2.0f;

        //const float x = 0.0f;
        const float x = mono::Random(-half_width, half_width);
        const float y = mono::Random(-half_height, half_height);

        const float velocity_x = mono::Random(-100.0f, -60.0f);
        const float velocity_y = mono::Random(-20.0f, 20.0f);

        //const float sign = mono::RandomInt(0, 1) == 0 ? 1.0f : -1.0f;
        //const float angular_velocity = mono::Random(0.3f, 1.0f) * sign;

        const float life = mono::Random(0.0f, 0.5f) + 10.0f;

        component_view.position = position + math::Vector(x, y);
        component_view.velocity = math::Vector(velocity_x, velocity_y);
        //pool.rotation = mono::Random(0.0f, math::PI() * 2.0f);
        //pool.angular_velocity = angular_velocity;

        using namespace mono::Color;

        component_view.gradient = MakeGradient<4>(
            { 0.0f, 0.5f, 1.0f, 1.0f },
            { RGBA(1.0f, 1.0f, 0.0f, 0.0f), RGBA(0.0f, 1.0f, 0.0f, 1.0f), RGBA(0.0f, 1.0f, 0.0f, 0.0f), RGBA() }
        );
        component_view.start_size = mono::Random(158.0f, 176.0f);
        component_view.end_size = mono::Random(2.0f, 6.0f);
        component_view.start_life = life;
        component_view.life = life;
    }
}

ScreenSparkles::ScreenSparkles(
    mono::ParticleSystem* particle_system,
    mono::IEntityManager* entity_system,
    const math::Vector& camera_position,
    const math::Vector& viewport_size)
    : m_particle_system(particle_system)
    , m_entity_system(entity_system)
{
    const float x = camera_position.x + viewport_size.x / 2.0f + 1.0f;
    const float y = camera_position.y;

    mono::Entity sparkles_entity = m_entity_system->CreateEntity("screensparkles", {});
    particle_system->AllocatePool(sparkles_entity.id, 1000, mono::DefaultUpdater);

    //const mono::ITexturePtr texture = mono::GetTextureFactory()->CreateTexture("res/textures/particles/x4.png");
    const mono::ITexturePtr texture = mono::GetTextureFactory()->CreateTexture("res/textures/particles/heart.png");
    //const mono::ITexturePtr texture = mono::GetTextureFactory()->CreateTexture("res/textures/particles/smoke_white_4.png");
    particle_system->SetPoolDrawData(sparkles_entity.id, texture, mono::BlendMode::ONE);

    const auto generator_proxy = [viewport_size](const math::Vector& position, mono::ParticlePoolComponentView& component_view) {
        Generator(position, component_view, viewport_size);
    };
    particle_system->AttachEmitter(sparkles_entity.id, math::Vector(x, y), -1.0f, 100.0f, mono::EmitterType::CONTINOUS, generator_proxy);

    m_particle_entity = sparkles_entity.id;
}

ScreenSparkles::~ScreenSparkles()
{
    m_particle_system->ReleasePool(m_particle_entity);
    m_entity_system->ReleaseEntity(m_particle_entity);
}
