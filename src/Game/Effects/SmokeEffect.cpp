
#include "SmokeEffect.h"
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
    void SmokeGenerator(const math::Vector& position, mono::ParticlePool& pool, size_t index, const void* context)
    {
        const float x = mono::Random(-2.0f, 2.0f);
        const float y = mono::Random(-2.0f, 2.0f);

        const int life = mono::RandomInt(0, 500) + 2000;

        pool.m_position[index] = position;
        pool.m_velocity[index] = math::Vector(x, y);
        pool.m_start_color[index] = mono::Color::RGBA(1.0f, 0.0f, 0.0f, 1.0f);
        pool.m_end_color[index] = mono::Color::RGBA(0.0f, 1.0f, 0.0f, 0.1f);
        pool.m_start_size[index] = 64.0f;
        pool.m_end_size[index] = 1.0f;
        pool.m_start_life[index] = life;
        pool.m_life[index] = life;
    }
}

SmokeEffect::SmokeEffect(const math::Vector& position)
{
    m_position = position;

    mono::ParticleEmitter::Configuration emit_config;
    //emit_config.position = position;
    emit_config.generator = SmokeGenerator;
    emit_config.emit_rate = 0.1f;

    const mono::ITexturePtr texture = mono::CreateTexture("res/textures/smoke.png");

    m_pool = std::make_unique<mono::ParticlePool>(1000, mono::DefaultUpdater, nullptr);
    m_emitter = std::make_unique<mono::ParticleEmitter>(emit_config, *m_pool);
    m_drawer = std::make_unique<mono::ParticleDrawer>(texture, *m_pool);
}

SmokeEffect::~SmokeEffect()
{ }

void SmokeEffect::Draw(mono::IRenderer& renderer) const
{
    m_drawer->doDraw(renderer);
}

void SmokeEffect::Update(unsigned int delta)
{
    m_emitter->doUpdate(delta);
    m_pool->doUpdate(delta);
    //mRotation += math::ToRadians(0.1) * float(delta);
}
