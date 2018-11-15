
#include "Enemy.h"
#include "CollisionConfiguration.h"

#include "Rendering/Sprite/ISprite.h"
#include "Rendering/Sprite/SpriteFactory.h"

#include "Physics/IBody.h"
#include "Physics/IShape.h"
#include "Physics/CMFactory.h"

#include "Rendering/IRenderer.h"

using namespace game;

Enemy::Enemy(EnemySetup& setup)
{
    m_position = setup.position;
    m_scale = math::Vector(setup.size, setup.size);

    mono::IBodyPtr body = mono::PhysicsFactory::CreateBody(setup.mass, 1.0f);
    body->SetPosition(m_position);
    
    mono::IShapePtr shape = mono::PhysicsFactory::CreateShape(body, setup.size / 2.0f, math::ZeroVec);
    shape->SetCollisionFilter(CollisionCategory::ENEMY, ENEMY_MASK);
    
    m_physics.body = body;
    m_physics.shapes.push_back(shape);

    m_sprite = mono::CreateSprite(setup.sprite_file);
    m_controller = std::move(setup.controller);
    m_controller->Initialize(this);
}

Enemy::~Enemy()
{ }

void Enemy::Draw(mono::IRenderer& renderer) const
{
    renderer.DrawSprite(*m_sprite);
}

void Enemy::Update(unsigned int delta)
{
    m_sprite->doUpdate(delta);
    m_controller->doUpdate(delta);
}
