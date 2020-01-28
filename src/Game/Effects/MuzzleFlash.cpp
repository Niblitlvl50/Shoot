
#include "MuzzleFlash.h"

//#include "Particle/ParticlePool.h"
//#include "Particle/ParticleEmitter.h"
//#include "Particle/ParticleDrawer.h"
//#include "Particle/ParticleSystemDefaults.h"
#include "Rendering/Texture/TextureFactory.h"
#include "Util/Random.h"

#include "Math/MathFunctions.h"

using namespace game;

MuzzleFlash::MuzzleFlash(const math::Vector& position)
{
    //m_position = position;

/*
    mono::ParticleEmitter::Configuration emit_config;
    emit_config.position = position;
    emit_config.generator = mono::DefaultGenerator;
    emit_config.burst = false;
    emit_config.duration = -1.0f;
    emit_config.emit_rate = 5.0f;

    const mono::ITexturePtr texture = mono::CreateTexture("res/textures/smoke.png");

    m_pool = std::make_unique<mono::ParticlePool>(100, mono::DefaultUpdater);
    m_emitter = std::make_unique<mono::ParticleEmitter>(emit_config, m_pool.get());
    m_drawer = std::make_unique<mono::ParticleDrawer>(texture, mono::BlendMode::ONE, *m_pool);
    */
}

MuzzleFlash::~MuzzleFlash()
{ }

void MuzzleFlash::Draw(mono::IRenderer& renderer) const
{
    //m_drawer->doDraw(renderer);
}

void MuzzleFlash::Update(const mono::UpdateContext& update_context)
{
    //m_emitter->doUpdate(update_context);
}

void MuzzleFlash::SetPosition(const math::Vector& position)
{
    //m_emitter->SetPosition(position);
}

