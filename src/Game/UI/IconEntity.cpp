
#include "IconEntity.h"
#include "Rendering/Sprite/SpriteFactory.h"
#include "Rendering/Sprite/ISprite.h"
#include "Rendering/IRenderer.h"

using namespace game;

IconEntity::IconEntity(const char* sprite_file)
{
    m_sprite = mono::CreateSprite(sprite_file);
}

void IconEntity::Update(unsigned int delta)
{
    m_sprite->doUpdate(delta);
}

void IconEntity::Draw(mono::IRenderer& renderer) const
{
    renderer.DrawSprite(*m_sprite);
}
