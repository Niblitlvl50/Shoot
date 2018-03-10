
#include "TrailEffect.h"
#include "Particle/ParticlePool.h"
#include "Particle/ParticleEmitter.h"
#include "Particle/ParticleDrawer.h"
#include "Particle/ParticleSystemDefaults.h"
#include "Rendering/Texture/TextureFactory.h"
#include "Rendering/IRenderer.h"
#include "Math/Matrix.h"
#include "Math/Quad.h"

#include <limits>

using namespace game;

namespace
{
    void TrailGenerator(const math::Vector& position, mono::ParticlePool& pool, size_t index, const void* context)
    {
        constexpr int life = 500;

        pool.m_position[index] = position;
        pool.m_start_color[index] = mono::Color::RGBA(1.0f, 0.0f, 0.0f, 1.0f);
        pool.m_end_color[index] = mono::Color::RGBA(1.0f, 0.0f, 0.0f, 0.1f);
        pool.m_start_size[index] = 16.0f;
        pool.m_end_size[index] = 10.0f;
        pool.m_start_life[index] = life;
        pool.m_life[index] = life;
    }
}

TrailEffect::TrailEffect(const math::Vector& position)
    : m_position(position)
{
    mono::ParticleEmitter::Configuration config;
    //config.position = position;
    config.generator = TrailGenerator;
    config.emit_rate = 0.2f;
    
    mono::ParticleDrawer::Configuration draw_config;
    draw_config.texture = mono::CreateTexture("res/textures/flare.png");

    m_pool = std::make_unique<mono::ParticlePool>(1000, mono::DefaultUpdater, nullptr);
    m_emitter = std::make_unique<mono::ParticleEmitter>(config, *m_pool);
    m_drawer = std::make_unique<mono::ParticleDrawer>(draw_config, *m_pool);
}

TrailEffect::~TrailEffect()
{ }

void TrailEffect::Draw(mono::IRenderer& renderer) const
{
    renderer.PushGlobalTransform();
    m_drawer->doDraw(renderer);
}

void TrailEffect::Update(unsigned int delta)
{
    m_emitter->doUpdate(delta);
    m_emitter->SetPosition(m_position);
    m_pool->doUpdate(delta);
}

math::Quad TrailEffect::BoundingBox() const
{
    constexpr float inf = std::numeric_limits<float>::infinity();
    return math::Quad(-inf, -inf, inf, inf);
}
