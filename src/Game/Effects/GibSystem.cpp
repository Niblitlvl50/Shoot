
#include "GibSystem.h"

//#include "Particle/ParticlePool.h"
//#include "Particle/ParticleEmitter.h"
//#include "Particle/ParticleDrawer.h"

//#include "Particle/ParticleSystemDefaults.h"
//#include "Rendering/Texture/TextureFactory.h"
#include "Util/Algorithm.h"
#include "Util/Random.h"

#include "Math/MathFunctions.h"
#include "Math/Quad.h"

#include "Actions/EasingFunctions.h"

using namespace game;

/*
namespace
{
    void GibsGenerator(const math::Vector& position, mono::ParticlePool& pool, size_t index, float direction)
    {
        constexpr float ten_degrees = math::ToRadians(10.0f);

        const float direction_variation = mono::Random(-ten_degrees, ten_degrees);
        const math::Vector& velocity = math::VectorFromAngle(direction + direction_variation);

        const int life = mono::RandomInt(3000, 3500);
        const float velocity_variation = mono::Random(2.0f, 16.0f);
        const float size = mono::Random(15.0f, 25.0f);

        pool.m_position[index] = position;
        pool.m_rotation[index] = 0.0f;
        pool.m_velocity[index] = velocity * velocity_variation;
        pool.m_start_color[index] = mono::Color::RGBA(0.5f, 0.0f, 0.0f, 1.0f);
        pool.m_end_color[index] = mono::Color::RGBA(0.5f, 0.0f, 0.0f, 0.1f);
        pool.m_start_size[index] = size;
        pool.m_end_size[index] = size;
        pool.m_size[index] = size;
        pool.m_start_life[index] = life;
        pool.m_life[index] = life;
    }

    void GibsUpdater(mono::ParticlePool& pool, size_t count, unsigned int delta)
    {
        const float float_delta = float(delta) / 1000.0f;

        for(size_t index = 0; index < count; ++index)
        {
            const float t = 1.0f - float(pool.m_life[index]) / float(pool.m_start_life[index]);
            const float t2 = float(pool.m_start_life[index]) - float(pool.m_life[index]);
            const float duration = float(pool.m_start_life[index]); // / 1000.0f;

            pool.m_velocity[index] *= 0.90;
            pool.m_position[index] += pool.m_velocity[index] * float_delta;
            //pool.m_size[index] = pool.m_start_size[index];

            const float alpha1 = pool.m_start_color[index].alpha;
            const float alpha2 = pool.m_end_color[index].alpha;
            
            pool.m_color[index] = mono::Color::LerpRGB(pool.m_start_color[index], pool.m_end_color[index], t);
            pool.m_color[index].alpha = game::EaseInCubic(t2, duration, alpha1, alpha2 - alpha1);
        }
    }
}
*/

GibSystem::GibSystem()
{
    //const mono::ITexturePtr texture = mono::CreateTexture("res/textures/flare.png");
    //m_pool = std::make_unique<mono::ParticlePool>(1500, GibsUpdater);
    //m_drawer = std::make_unique<mono::ParticleDrawer>(texture, mono::BlendMode::ONE, *m_pool);
}

GibSystem::~GibSystem()
{ }

void GibSystem::Update(const mono::UpdateContext& update_context)
{
    /*
    for(auto&& emitter : m_emitters)
        emitter.Update(update_context);

    m_pool->Update(update_context);

    const auto remove_func = [](const mono::ParticleEmitter& emitter) {
        return emitter.IsDone();
    };

    mono::remove_if(m_emitters, remove_func);
    */
}

void GibSystem::Draw(mono::IRenderer& renderer) const
{
    //m_drawer->Draw(renderer);
}

math::Quad GibSystem::BoundingBox() const
{
    return math::InfQuad;
}

void GibSystem::EmitGibsAt(const math::Vector& position, float direction)
{
    /*
    mono::ParticleEmitter::Configuration emit_config;
    emit_config.position = position;
    emit_config.burst = true;
    emit_config.duration = 1.0f;
    emit_config.emit_rate = 30.0f;
    emit_config.generator = [direction](const math::Vector& position, mono::ParticlePool& pool, size_t index) {
        GibsGenerator(position, pool, index, direction);
    };

    m_emitters.emplace_back(emit_config, m_pool.get());
    */
}

void GibSystem::EmitBloodAt(const math::Vector& position, float direction)
{

}

