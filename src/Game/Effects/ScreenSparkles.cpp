
#include "ScreenSparkles.h"
#include "Particle/ParticlePool.h"
#include "Particle/ParticleEmitter.h"
#include "Particle/ParticleDrawer.h"
#include "Particle/ParticleSystemDefaults.h"
#include "Rendering/Texture/TextureFactory.h"
#include "Random.h"

#include "Math/MathFunctions.h"
#include "Math/Quad.h"

using namespace game;

namespace
{
    void Generator(const math::Vector& position, mono::ParticlePool& pool, size_t index, const void* context)
    {
        const math::Quad* viewport = static_cast<const math::Quad*>(context);
        const float half_height = viewport->mB.y / 2.0f;

        const float x = 0.0f;
        const float y = mono::Random(-half_height, half_height);

        const float velocity_x = mono::Random(-20.0f, -10.0f);
        const float velocity_y = mono::Random(-2.0f, 2.0f);

        const int life = mono::RandomInt(0, 500) + 3000;

        pool.m_position[index] = position + math::Vector(x, y);
        pool.m_velocity[index] = math::Vector(velocity_x, velocity_y);
        pool.m_start_color[index] = mono::Color::RGBA(0.8f, 0.8f, 1.0f, 1.0f);
        pool.m_end_color[index] = mono::Color::RGBA(0.0f, 0.0f, 1.0f, 0.1f);
        pool.m_start_size[index] = mono::Random(8.0f, 16.0f);
        pool.m_end_size[index] = mono::Random(2.0f, 6.0f);
        pool.m_start_life[index] = life;
        pool.m_life[index] = life;
    }
}

ScreenSparkles::ScreenSparkles(const math::Quad& viewport, float particle_size)
    : m_viewport(viewport)
{
    const float x = viewport.mB.x;
    const float y = viewport.mB.y / 2.0f;

    m_position = math::Vector(x, y);

    mono::ParticleEmitter::Configuration emit_config;
    //emit_config.position = math::Vector(200, 100);
    emit_config.emit_rate = 0.2f;
    emit_config.generator = Generator;
    emit_config.generator_context = &m_viewport;

    const mono::ITexturePtr texture = mono::CreateTexture("res/textures/flare.png");

    m_pool = std::make_unique<mono::ParticlePool>(500, mono::DefaultUpdater, nullptr);
    m_emitter = std::make_unique<mono::ParticleEmitter>(emit_config, *m_pool);
    m_drawer = std::make_unique<mono::ParticleDrawer>(texture, *m_pool);
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
}
