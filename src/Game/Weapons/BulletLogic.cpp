
#include "BulletLogic.h"
#include "Audio/ISound.h"
#include "Audio/AudioFactory.h"
#include "Util/Random.h"

using namespace game;

BulletLogic::BulletLogic(uint32_t entity_id, const BulletConfiguration& config)
    : m_entity_id(entity_id)
    , m_collision_callback(config.collision_callback)
{
    const float life_span = config.life_span + (mono::Random() * config.fuzzy_life_span);
    m_life_span = life_span * 1000.0f;

    if(config.sound_file)
    {
        m_sound = mono::AudioFactory::CreateSound(config.sound_file, true, false);
        m_sound->Play();
    }
    else
    {
        m_sound = mono::AudioFactory::CreateNullSound();
    }
}

void BulletLogic::Update(uint32_t delta_ms)
{
    m_sound->Position(0.0f, 0.0f);

    m_life_span -= delta_ms;
    if(m_life_span < 0)
        m_collision_callback(m_entity_id, nullptr);
}

void BulletLogic::OnCollideWith(mono::IBody* body, const math::Vector& collision_point, uint32_t categories)
{
    m_collision_callback(m_entity_id, body);
}
