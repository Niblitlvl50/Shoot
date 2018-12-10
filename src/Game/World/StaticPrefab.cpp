
#include "StaticPrefab.h"
#include "CollisionConfiguration.h"

#include "Math/Matrix.h"

#include "Physics/IBody.h"
#include "Physics/IShape.h"
#include "Physics/CMFactory.h"
#include "Physics/CMPhysicsData.h"

#include "Rendering/IRenderer.h"
#include "Rendering/Sprite/Sprite.h"
#include "Rendering/Sprite/SpriteFactory.h"


StaticPrefab::StaticPrefab(
    const math::Vector& position,
    const char* sprite_file,
    const std::vector<math::Vector>& collision_polygon)
{
    m_position = position;

    m_physics.body = mono::PhysicsFactory::CreateStaticBody();
    m_physics.body->SetPosition(m_position);

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

void StaticPrefab::Update(unsigned int delta)
{ }

math::Quad StaticPrefab::BoundingBox() const
{
    const mono::SpriteFrame& current_frame = m_sprite->GetCurrentFrame();
    const math::Vector& sprite_size = current_frame.size / 2.0f;
    const math::Matrix& transform = Transformation();
 
    return math::Transform(transform, math::Quad(-sprite_size, sprite_size));
}
