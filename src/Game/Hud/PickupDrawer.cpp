
#include "PickupDrawer.h"
#include "Math/Quad.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Sprite/ISpriteFactory.h"
#include "Rendering/Sprite/ISprite.h"

using namespace game;

PickupDrawer::PickupDrawer(const std::vector<Pickup>& ammo_pickups)
    : m_ammo_pickups(ammo_pickups)
{
    m_pickup_sprites[PickupTypes::AMMO] = mono::GetSpriteFactory()->CreateSprite("res/sprites/ammo_pickup.sprite");
    m_pickup_sprites[PickupTypes::HEALTH] = mono::GetSpriteFactory()->CreateSprite("res/sprites/health_pickup.sprite");
}

PickupDrawer::~PickupDrawer()
{ }

void PickupDrawer::Draw(mono::IRenderer& renderer) const
{
    for(const Pickup& ammo : m_ammo_pickups)
        renderer.DrawSprite(*m_pickup_sprites[ammo.type], ammo.position);
}

math::Quad PickupDrawer::BoundingBox() const
{
    return math::InfQuad;
}
