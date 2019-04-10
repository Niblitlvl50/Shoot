
#pragma once

#include "Entity/PhysicsEntityBase.h"
#include "Rendering/RenderPtrFwd.h"
#include "Rendering/Sprite/ISpriteFactory.h"

class StaticPrefab : public mono::PhysicsEntityBase
{
public:

    StaticPrefab(
        const math::Vector& position,
        const char* sprite_file,
        const std::vector<math::Vector>& collision_polygon);
    ~StaticPrefab();

    void Draw(mono::IRenderer& renderer) const override;
    void Update(unsigned int delta) override;
    math::Quad BoundingBox() const override;

    mono::ISpritePtr m_sprite;
};
