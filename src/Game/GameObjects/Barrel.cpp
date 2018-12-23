
#include "Barrel.h"
#include "CollisionConfiguration.h"

#include <Math/Matrix.h>

#include <Rendering/IRenderer.h>
#include <Rendering/Sprite/ISprite.h>
#include <Rendering/Sprite/SpriteFactory.h>

#include <Physics/IBody.h>
#include <Physics/IShape.h>
#include <Physics/CMFactory.h>

using namespace game;

Barrel::Barrel(const Configuration& config)
{
    m_sprite = mono::CreateSprite(config.sprite_file.c_str());
    m_scale = config.scale;

    const math::Vector& size = m_sprite->GetCurrentFrame().size;

    auto body = mono::PhysicsFactory::CreateBody(config.mass, math::INF);
    auto shape = mono::PhysicsFactory::CreateShape(body, size.x, size.y);
    shape->SetCollisionFilter(CollisionCategory::PROPS, PROPS_MASK);

    m_physics.body = body;
    m_physics.shapes.push_back(shape);
}

Barrel::~Barrel()
{ }

void Barrel::Update(unsigned int delta)
{
    m_sprite->doUpdate(delta);
}

void Barrel::Draw(mono::IRenderer& renderer) const
{
    renderer.DrawSprite(*m_sprite);
}

math::Quad Barrel::BoundingBox() const
{
    const mono::SpriteFrame& current_frame = m_sprite->GetCurrentFrame();
    const math::Vector& sprite_size = current_frame.size / 2.0f;
    const math::Matrix& transform = Transformation();
 
    return math::Transform(transform, math::Quad(-sprite_size, sprite_size));
}
