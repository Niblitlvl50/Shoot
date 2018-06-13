
#include "ParticleExplosion.h"
#include "Actions/EasingFunctions.h"

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
    void SmokeGenerator(const math::Vector& position, mono::ParticlePool& pool, size_t index)
    {
        const float x = mono::Random(-1.0f, 1.0f);
        const float y = mono::Random(-1.0f, 1.0f);
        const int life = mono::RandomInt(1500, 2000);

        pool.m_position[index] = position;
        pool.m_velocity[index] = math::Vector(x, y);
        pool.m_start_color[index] = mono::Color::RGBA(0.8f, 0.8f, 0.8f, 1.0f);
        pool.m_end_color[index] = mono::Color::RGBA(0.6f, 0.6f, 0.6f, 1.0f);
        pool.m_start_size[index] = 164.0f;
        pool.m_end_size[index] = 100.0f;
        pool.m_start_life[index] = life;
        pool.m_life[index] = life;
    }

    void SmokeUpdater(mono::ParticlePool& pool, size_t count, unsigned int delta)
    {
        const float float_delta = float(delta) / 1000.0f;

        for(size_t index = 0; index < count; ++index)
        {
            const float t = 1.0f - float(pool.m_life[index]) / float(pool.m_start_life[index]);

            const math::Vector& velocity = pool.m_velocity[index];
            const float new_x = game::EaseOutCubic(t, float(pool.m_start_life[index]), velocity.x, 0.0f);
            const float new_y = game::EaseOutCubic(t, float(pool.m_start_life[index]), velocity.y, 0.0f);

            pool.m_position[index] += math::Vector(new_x, new_y) * float_delta;
            pool.m_color[index] = mono::Color::Lerp(pool.m_start_color[index], pool.m_end_color[index], t);
            pool.m_size[index] = (1.0f - t) * pool.m_start_size[index] + t * pool.m_end_size[index];
        }        
    }

    void FlareGenerator(const math::Vector& position, mono::ParticlePool& pool, size_t index)
    {
        const float speed = mono::Random(3.0f, 4.0f);
        const float direction = mono::Random(0.0f, math::PI() * 2.0f);
        const int life = mono::RandomInt(500, 1000);

        pool.m_position[index] = position;
        pool.m_velocity[index] = math::VectorFromAngle(direction) * speed;
        pool.m_start_color[index] = mono::Color::RGBA(1.0f, 0.0f, 0.0f, 1.0f);
        pool.m_end_color[index] = mono::Color::RGBA(0.7f, 0.0f, 0.0f, 0.0f);
        pool.m_start_size[index] = 32.0f;
        pool.m_end_size[index] = 32.0f;
        pool.m_start_life[index] = life;
        pool.m_life[index] = life;
    }

    void FlareGenerator2(const math::Vector& position, mono::ParticlePool& pool, size_t index)
    {
        const float x = mono::Random(-2.0f, 2.0f);
        const float y = mono::Random(-2.0f, 2.0f);
        const int life = mono::RandomInt(0, 500) + 2000;

        pool.m_position[index] = position;
        pool.m_velocity[index] = math::Vector(x, y);
        pool.m_start_color[index] = mono::Color::RGBA(0.0f, 0.0f, 1.0f, 0.8f);
        pool.m_end_color[index] = mono::Color::RGBA(0.0f, 0.0f, 0.6f, 0.0f);
        pool.m_start_size[index] = 128.0f;
        pool.m_end_size[index] = 64.0f;
        pool.m_start_life[index] = life;
        pool.m_life[index] = life;
    }
}

ParticleExplosion::ParticleExplosion(const math::Vector& position)
{
    m_position = position;

    mono::ParticleEmitter::Configuration smoke_config;
    smoke_config.generator = SmokeGenerator;
    smoke_config.burst = true;
    smoke_config.duration = 1.0f;
    smoke_config.emit_rate = 10.0f;

    mono::ParticleEmitter::Configuration flare_config;
    flare_config.generator = FlareGenerator;
    flare_config.burst = true;
    flare_config.duration = 1.0f;
    flare_config.emit_rate = 10.0f;

    mono::ParticleEmitter::Configuration emit_config;
    emit_config.generator = FlareGenerator2;
    emit_config.burst = true;
    emit_config.duration = 1.0f;
    emit_config.emit_rate = 5.0f;

    const mono::ITexturePtr smoke_texture = mono::CreateTexture("res/textures/smoke.png");
    const mono::ITexturePtr flare_texture = mono::CreateTexture("res/textures/flare.png");
    const mono::ITexturePtr texture3 = mono::CreateTexture("res/textures/smoke.png");

    m_pool1 = std::make_unique<mono::ParticlePool>(100, SmokeUpdater);
    m_pool2 = std::make_unique<mono::ParticlePool>(100, mono::DefaultUpdater);
    m_pool3 = std::make_unique<mono::ParticlePool>(100, mono::DefaultUpdater);

    m_emitter1 = std::make_unique<mono::ParticleEmitter>(smoke_config, m_pool1.get());
    m_emitter2 = std::make_unique<mono::ParticleEmitter>(flare_config, m_pool2.get());
    m_emitter3 = std::make_unique<mono::ParticleEmitter>(emit_config, m_pool3.get());

    m_drawer1 = std::make_unique<mono::ParticleDrawer>(smoke_texture, mono::BlendMode::ONE, *m_pool1);
    m_drawer2 = std::make_unique<mono::ParticleDrawer>(flare_texture, mono::BlendMode::ONE, *m_pool2);
    m_drawer3 = std::make_unique<mono::ParticleDrawer>(texture3, mono::BlendMode::ONE, *m_pool3);
}

ParticleExplosion::~ParticleExplosion()
{ }

void ParticleExplosion::Draw(mono::IRenderer& renderer) const
{
    m_drawer1->doDraw(renderer);
    //m_drawer2->doDraw(renderer);
    //m_drawer3->doDraw(renderer);
}

void ParticleExplosion::Update(unsigned int delta)
{
    m_emitter1->doUpdate(delta);
    m_emitter2->doUpdate(delta);
    m_emitter3->doUpdate(delta);

    m_pool1->doUpdate(delta);
    m_pool2->doUpdate(delta);
    m_pool3->doUpdate(delta);
}
