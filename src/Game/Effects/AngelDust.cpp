
#include "AngelDust.h"
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
    void Generator(const math::Vector& position, mono::ParticlePoolComponentView& pool, const math::Quad& area)
    {
        const float x = mono::Random(area.mA.x, area.mB.x);
        const float y = mono::Random(area.mA.y, area.mB.y);

        const float velocity_x = mono::Random(-1.0f, 1.0f);
        const float velocity_y = mono::Random(-0.25f, 0.25f);

        const int life = mono::RandomInt(0, 500) + 5000;

        pool.position = position + math::Vector(x, y);
        pool.velocity = math::Vector(velocity_x, velocity_y);
        //pool.rotation[index] = mono::Random(0.0f, math::PI() * 2.0f);
        //pool.angular_velocity[index] = angular_velocity;

        using namespace mono::Color;

        pool.gradient = mono::Color::MakeGradient<4>(
            { 0.0f, 0.5f, 1.0f, 1.0f },
            { RGBA(1.0f, 1.0f, 0.0f, 0.0f), RGBA(1.0f, 1.0f, 0.0f, 0.25f), RGBA(1.0f, 1.0f, 0.0f, 0.0f), RGBA() }
        );

        pool.start_size = 64.0f;
        pool.end_size = 16.0f;
        pool.start_life = life;
        pool.life = life;
    }
}

AngelDust::AngelDust(mono::ParticleSystem* particle_system, mono::IEntityManager* entity_system, const math::Quad& area)
    : m_particle_system(particle_system)
    , m_entity_system(entity_system)
{
    mono::Entity sparkles_entity = m_entity_system->CreateEntity("angeldust", {});
    particle_system->AllocatePool(sparkles_entity.id, 1000, mono::DefaultUpdater);

    const mono::ITexturePtr texture = mono::GetTextureFactory()->CreateTexture("res/textures/particles/flare.png");
    particle_system->SetPoolDrawData(sparkles_entity.id, texture, mono::BlendMode::ONE);

    const auto generator_proxy = [area](const math::Vector& position, mono::ParticlePoolComponentView& pool) {
        Generator(position, pool, area);
    };
    particle_system->AttachEmitter(sparkles_entity.id, math::ZeroVec, -1.0f, 200.0f, mono::EmitterType::CONTINOUS, generator_proxy);

    m_particle_entity = sparkles_entity.id;
}

AngelDust::~AngelDust()
{
    m_particle_system->ReleasePool(m_particle_entity);
    m_entity_system->ReleaseEntity(m_particle_entity);
}
