
#include "SpriteEntity.h"
#include "Rendering/Sprite/ISprite.h"
#include "Rendering/Sprite/SpriteFactory.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"
#include "Math/Quad.h"

using namespace editor;

SpriteEntity::SpriteEntity(const char* name, const char* sprite_file)
    : m_name(name),
      m_selected(false)
{
    m_sprite = mono::CreateSprite(sprite_file);
}

SpriteEntity::~SpriteEntity()
{ }

void SpriteEntity::Draw(mono::IRenderer& renderer) const
{
    renderer.DrawSprite(*m_sprite);

    if(m_selected)
    {
        math::Quad bb(-0.5f, -0.5f, 0.5f, 0.5f);
        renderer.DrawQuad(bb, mono::Color::RGBA(0.0f, 1.0f, 0.0f), 2.0f);
    }
}

void SpriteEntity::Update(unsigned int delta)
{
    m_sprite->doUpdate(delta);
}

void SpriteEntity::SetSelected(bool selected)
{
    m_selected = selected;
}

void SpriteEntity::SetSpriteAnimation(int sprite_id)
{
    m_sprite->SetAnimation(sprite_id);
}

const std::string& SpriteEntity::Name() const
{
    return m_name;
}
