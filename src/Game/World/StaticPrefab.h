
#pragma once

#include "Entity/PhysicsEntityBase.h"
#include "Rendering/RenderPtrFwd.h"

class StaticPrefab : public mono::PhysicsEntityBase
{
public:

    StaticPrefab(
        const math::Vector& position,
        const math::Vector& scale,
        const char* sprite_file,
        const std::vector<math::Vector>& collision_polygon);

    void Draw(mono::IRenderer& renderer) const override;

    // Overriden so that we dont get any updates that screws up the position
    void doUpdate(unsigned int delta) override;
    void Update(unsigned int delta) override;

    mono::ISpritePtr m_sprite;
};
