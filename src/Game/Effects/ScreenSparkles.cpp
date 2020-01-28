
#include "ScreenSparkles.h"
//#include "Particle/ParticlePool.h"
//#include "Particle/ParticleEmitter.h"
//#include "Particle/ParticleDrawer.h"
//#include "Particle/ParticleSystemDefaults.h"
#include "Rendering/Texture/TextureFactory.h"
#include "Util/Random.h"

#include "Math/MathFunctions.h"
#include "Math/Quad.h"

using namespace game;

/*
namespace
{
    void Generator(const math::Vector& position, mono::ParticlePool& pool, size_t index, const math::Quad& viewport)
    {
        const float half_height = viewport.mB.y / 2.0f;

        const float x = 0.0f;
        const float y = mono::Random(-half_height, half_height);

        const float velocity_x = mono::Random(-5.0f, -3.0f);
        const float velocity_y = mono::Random(-1.0f, 1.0f);

        const float sign = mono::RandomInt(0, 1) == 0 ? 1.0f : -1.0f;
        const float angular_velocity = mono::Random(0.3f, 1.0f) * sign;

        const int life = mono::RandomInt(0, 500) + 5000;

        pool.m_position[index] = position + math::Vector(x, y);
        pool.m_velocity[index] = math::Vector(velocity_x, velocity_y);
        pool.m_rotation[index] = mono::Random(0.0f, math::PI() * 2.0f);
        pool.m_angular_velocity[index] = angular_velocity;
        pool.m_start_color[index] = mono::Color::RGBA(1.0f, 0.0f, 0.0f, 1.0f);
        pool.m_end_color[index] = mono::Color::RGBA(0.0f, 1.0f, 0.0f, 1.0f);
        pool.m_start_size[index] = mono::Random(58.0f, 76.0f);
        pool.m_end_size[index] = mono::Random(2.0f, 6.0f);
        pool.m_start_life[index] = life;
        pool.m_life[index] = life;
    }

    void SparklesUpdater(mono::ParticlePool& pool, size_t count, unsigned int delta)
    {
        const float float_delta = float(delta) / 1000.0f;

        for(size_t index = 0; index < count; ++index)
        {
            const float t = 1.0f - float(pool.m_life[index]) / float(pool.m_start_life[index]);

            pool.m_position[index] += pool.m_velocity[index] * float_delta;
            pool.m_color[index] = mono::Color::Lerp(pool.m_start_color[index], pool.m_end_color[index], t);
            pool.m_size[index] = (1.0f - t) * pool.m_start_size[index] + t * pool.m_end_size[index];
            pool.m_rotation[index] += pool.m_angular_velocity[index] * float_delta;
        }
    }
}
*/

ScreenSparkles::ScreenSparkles(const math::Quad& viewport)
    : m_viewport(viewport)
{
    const float x = viewport.mB.x;
    const float y = viewport.mB.y / 2.0f;

    m_position = math::Vector(x, y);

/*
    mono::ParticleEmitter::Configuration emit_config;
    emit_config.duration = -1.0f;
    emit_config.emit_rate = 100.0f;
    emit_config.burst = false;
    emit_config.generator = [viewport](const math::Vector& position, mono::ParticlePool& pool, size_t index) {
        Generator(position, pool, index, viewport);
    };

    m_pool = std::make_unique<mono::ParticlePool>(500, SparklesUpdater);
    m_emitter = std::make_unique<mono::ParticleEmitter>(emit_config, m_pool.get());

    const mono::ITexturePtr texture = mono::CreateTexture("res/textures/x4.png");
    m_drawer = std::make_unique<mono::ParticleDrawer>(texture, mono::BlendMode::SOURCE_ALPHA, *m_pool);
    */
}

ScreenSparkles::~ScreenSparkles()
{ }

void ScreenSparkles::Draw(mono::IRenderer& renderer) const
{
    //m_drawer->doDraw(renderer);
}

void ScreenSparkles::Update(const mono::UpdateContext& update_context)
{
    //m_emitter->doUpdate(update_context);
    //m_pool->Update(update_context);
}
