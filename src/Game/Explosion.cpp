
#include "Explosion.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Sprite/ISprite.h"
#include "Rendering/Sprite/SpriteFactory.h"
#include "Events/RemoveEntityEvent.h"
#include "EventHandler/EventHandler.h"
#include "Audio/ISound.h"
#include "Audio/AudioFactory.h"

using namespace game;

Explosion::Explosion(const ExplosionConfiguration& config, mono::EventHandler& event_handler)
{
    m_position = config.position;
    m_scale = math::Vector(config.scale, config.scale);
    m_rotation = config.rotation;

    if(config.sound_file)
    {
        m_sound = mono::AudioFactory::CreateSound(config.sound_file, false, true);
        m_sound->Position(m_position.x, m_position.y);
    }

    const auto func = [this, &event_handler] {
        if(m_sound)
            m_sound->Play();
        event_handler.DispatchEvent(game::RemoveEntityEvent(Id()));
    };
    
    m_sprite = mono::CreateSprite(config.sprite_file);
    m_sprite->SetAnimation(0, func);
}

void Explosion::Update(unsigned int delta)
{
    m_sprite->doUpdate(delta);
}

void Explosion::Draw(mono::IRenderer& renderer) const
{
    renderer.DrawSprite(*m_sprite);
}
