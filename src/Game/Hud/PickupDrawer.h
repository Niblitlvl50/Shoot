
#pragma once

#include "Rendering/IDrawable.h"
#include "Rendering/RenderPtrFwd.h"
#include "Pickups/Ammo.h"
#include <vector>

namespace game
{
    struct Ammo;

    class PickupDrawer : public mono::IDrawable
    {
    public:
    
        PickupDrawer(const std::vector<Ammo>& ammo_pickups);
        ~PickupDrawer();

        void doDraw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

        const std::vector<Ammo>& m_ammo_pickups;
        mono::ISpritePtr m_pickup_sprites[Pickups::N_AMMOS];
    };
}
