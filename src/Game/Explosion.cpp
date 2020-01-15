
#include "Explosion.h"

#include "Math/Matrix.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Sprite/ISprite.h"
#include "Rendering/Sprite/ISpriteFactory.h"
#include "EventHandler/EventHandler.h"
#include "Audio/ISound.h"
#include "Audio/AudioFactory.h"

using namespace game;

Explosion::Explosion(const ExplosionConfiguration& config, mono::EventHandler& event_handler)
{
    m_position = config.position;
    m_scale = math::Vector(config.scale, config.scale);
    m_rotation = config.rotation;

    const auto remove_this_func = [this, &event_handler] {
        //event_handler.DispatchEvent(game::RemoveEntityEvent(Id()));
    };

    m_sprite = mono::GetSpriteFactory()->CreateSprite(config.sprite_file);
    m_sprite->SetAnimation(0, remove_this_func);

    if(config.sound_file)
    {
        m_sound = mono::AudioFactory::CreateSound(config.sound_file, false, true);
        m_sound->Position(m_position.x, m_position.y);
        m_sound->Play();
    }
}

Explosion::~Explosion()
{ }

void Explosion::Update(const mono::UpdateContext& update_context)
{
    m_sprite->doUpdate(update_context);
}

void Explosion::Draw(mono::IRenderer& renderer) const
{
    renderer.DrawSprite(*m_sprite);
}

math::Quad Explosion::BoundingBox() const
{
    const mono::SpriteFrame& current_frame = m_sprite->GetCurrentFrame();
    const math::Vector& sprite_size = current_frame.size / 2.0f;
    const math::Matrix& transform = Transformation();
 
    return math::Transform(transform, math::Quad(-sprite_size, sprite_size));
}
