
#include "IconEntity.h"
#include "Rendering/Sprite/ISpriteFactory.h"
#include "Rendering/Sprite/ISprite.h"
#include "Rendering/IRenderer.h"

using namespace game;

IconEntity::IconEntity(const char* sprite_file)
{
    m_sprite = mono::GetSpriteFactory()->CreateSprite(sprite_file);
}

IconEntity::~IconEntity()
{ }

void IconEntity::Update(const mono::UpdateContext& update_context)
{
    m_sprite->doUpdate(update_context);
}

void IconEntity::Draw(mono::IRenderer& renderer) const
{
    renderer.DrawSprite(*m_sprite);
}
