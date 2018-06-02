
#pragma once

#include "Rendering/IDrawable.h"
#include <vector>

namespace game
{
    struct Ammo;

    class PickupDrawer : public mono::IDrawable
    {
    public:
    
        PickupDrawer(const std::vector<Ammo>& ammo_pickups);

        void doDraw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

        const std::vector<Ammo>& m_ammo_pickups;
    };
}
