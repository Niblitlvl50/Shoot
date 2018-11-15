
#include "Barrel.h"
#include "CollisionConfiguration.h"

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

    auto body = mono::PhysicsFactory::CreateBody(25.0f, math::INF);
    auto shape = mono::PhysicsFactory::CreateShape(body, m_scale.x, m_scale.y);
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
