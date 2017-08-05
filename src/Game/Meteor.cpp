
#include "Meteor.h"
#include "Rendering/IRenderer.h"
#include "Physics/IBody.h"
#include "Physics/IShape.h"
#include "Physics/CMFactory.h"

#include "EntityProperties.h"
#include "Rendering/Sprite/ISprite.h"
#include "Rendering/Sprite/SpriteFactory.h"

using namespace game;

Meteor::Meteor(float x, float y)
{
    mSprite = mono::CreateSprite("res/sprites/meteorite.sprite");

    m_position = math::Vector(x, y);
    m_scale = math::Vector(1.0f, 1.0f);

    SetProperty(EntityProperties::DAMAGABLE);
    
    m_physics.body = mono::PhysicsFactory::CreateBody(15.0f, 1.0f);
    m_physics.body->SetPosition(m_position);

    mono::IShapePtr shape = mono::PhysicsFactory::CreateShape(m_physics.body, m_scale.x * 0.5, math::zeroVec);
    shape->SetElasticity(0.9f);
    
    m_physics.body->SetMoment(shape->GetInertiaValue());
    m_physics.shapes.push_back(shape);
}

void Meteor::Draw(mono::IRenderer& renderer) const
{
    renderer.DrawSprite(*mSprite);
}

void Meteor::Update(unsigned int delta)
{ }
