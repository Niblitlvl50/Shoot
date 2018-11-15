
#include "StaticPrefab.h"
#include "CollisionConfiguration.h"

#include "Physics/IBody.h"
#include "Physics/IShape.h"
#include "Physics/CMFactory.h"
#include "Physics/CMPhysicsData.h"

#include "Rendering/IRenderer.h"
#include "Rendering/Sprite/Sprite.h"
#include "Rendering/Sprite/SpriteFactory.h"


StaticPrefab::StaticPrefab(
    const math::Vector& position,
    const math::Vector& scale,
    const char* sprite_file,
    const std::vector<math::Vector>& collision_polygon)
{
    m_position = position;
    m_scale = scale;

    m_physics.body = mono::PhysicsFactory::CreateStaticBody();

    mono::IShapePtr shape = mono::PhysicsFactory::CreateShape(m_physics.body, collision_polygon, math::ZeroVec);
    shape->SetCollisionFilter(game::CollisionCategory::STATIC, game::STATIC_MASK);
    m_physics.shapes.push_back(shape);

    m_sprite = mono::CreateSprite(sprite_file);
}

StaticPrefab::~StaticPrefab()
{ }

void StaticPrefab::Draw(mono::IRenderer& renderer) const
{
    renderer.DrawSprite(*m_sprite);
}

// Overriden so that we dont get any updates that screws up the position
void StaticPrefab::doUpdate(unsigned int delta)
{ }

void StaticPrefab::Update(unsigned int delta)
{ }
