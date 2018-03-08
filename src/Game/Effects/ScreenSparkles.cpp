
#include "ScreenSparkles.h"
#include "Particle/ParticlePool.h"
#include "Particle/ParticleEmitter.h"
#include "Particle/ParticleDrawer.h"
#include "Particle/ParticleSystemDefaults.h"
#include "Rendering/Texture/TextureFactory.h"
#include "Random.h"

#include "Math/MathFunctions.h"

using namespace game;

namespace
{
    void Generator(const math::Vector& position, mono::ParticlePool& pool, size_t index)
    {
        const float x = 0.0f; //mono::Random(-50.0f, 2.0f);
        const float y = mono::Random(-50.0f, 50.0f);

        const float velocity_x = mono::Random(-20.0f, -10.0f);
        const float velocity_y = mono::Random(-2.0f, 2.0f);

        const int life = mono::RandomInt(0, 500) + 2500;

        pool.m_position[index] = position + math::Vector(x, y);
        pool.m_velocity[index] = math::Vector(velocity_x, velocity_y);
        pool.m_startColor[index] = mono::Color::RGBA(0.8f, 0.8f, 1.0f, 1.0f);
        pool.m_endColor[index] = mono::Color::RGBA(0.0f, 0.0f, 1.0f, 0.1f);
        pool.m_startLife[index] = life;
        pool.m_life[index] = life;
    }
}

ScreenSparkles::ScreenSparkles(const math::Vector& position)
{
    m_position = position;

    mono::ParticleEmitter::Configuration emit_config;
    //emit_config.position = math::Vector(200, 100);
    emit_config.generator = Generator;
    emit_config.emit_rate = 0.2f;

    mono::ParticleDrawer::Configuration draw_config;
    draw_config.texture = mono::CreateTexture("res/textures/flare.png");
    draw_config.point_size = 12.0f;

    m_pool = std::make_unique<mono::ParticlePool>(1000, mono::DefaultUpdater);
    m_emitter = std::make_unique<mono::ParticleEmitter>(emit_config, *m_pool);
    m_drawer = std::make_unique<mono::ParticleDrawer>(draw_config, *m_pool);
}

ScreenSparkles::~ScreenSparkles()
{ }

void ScreenSparkles::Draw(mono::IRenderer& renderer) const
{
    m_drawer->doDraw(renderer);
}

void ScreenSparkles::Update(unsigned int delta)
{
    m_emitter->doUpdate(delta);
    m_pool->doUpdate(delta);
    //mRotation += math::ToRadians(0.1) * float(delta);
}
