
#include "Explosion.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Sprite/ISprite.h"
#include "Rendering/Sprite/SpriteFactory.h"
#include "Events/RemoveEntityEvent.h"
#include "EventHandler/EventHandler.h"

using namespace game;

Explosion::Explosion(const ExplosionConfiguration& config, mono::EventHandler& event_handler)
{
    m_position = config.position;
    m_scale = math::Vector(config.scale, config.scale);
    m_rotation = config.rotation;

    const unsigned int id = Id();

    const auto func = [&event_handler, id] {
        event_handler.DispatchEvent(game::RemoveEntityEvent(id));
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
