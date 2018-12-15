
#include "SpriteEntity.h"
#include "Rendering/Sprite/ISprite.h"
#include "Rendering/Sprite/SpriteFactory.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"

#include "Math/Matrix.h"
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
        const mono::SpriteFrame& current_frame = m_sprite->GetCurrentFrame();
        const math::Vector& sprite_size = current_frame.size / 2.0f;
        const math::Quad bb = math::Quad(-sprite_size, sprite_size);

        renderer.DrawQuad(bb, mono::Color::RGBA(0.0f, 1.0f, 0.0f), 2.0f);
    }
}

void SpriteEntity::Update(unsigned int delta)
{
    m_sprite->doUpdate(delta);
}

math::Quad SpriteEntity::BoundingBox() const
{
    const mono::SpriteFrame& current_frame = m_sprite->GetCurrentFrame();
    const math::Vector& sprite_size = current_frame.size / 2.0f;
    const math::Matrix& transform = Transformation();
 
    return math::Transform(transform, math::Quad(-sprite_size, sprite_size));
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
