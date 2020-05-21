
#pragma once

#include "Rendering/IDrawable.h"
#include "Rendering/RenderPtrFwd.h"
#include "Rendering/Sprite/ISpriteFactory.h"
#include "Pickups/Ammo.h"
#include <vector>

namespace game
{
    struct Pickup;

    class PickupDrawer : public mono::IDrawable
    {
    public:
    
        PickupDrawer(const std::vector<Pickup>& ammo_pickups);
        ~PickupDrawer();

        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

        const std::vector<Pickup>& m_ammo_pickups;
        mono::ISpritePtr m_pickup_sprites[PickupTypes::N_PICKUPS];
    };
}
