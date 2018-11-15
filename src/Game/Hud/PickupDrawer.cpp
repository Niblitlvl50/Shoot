
#include "PickupDrawer.h"
#include "Math/Quad.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Sprite/SpriteFactory.h"
#include "Rendering/Sprite/ISprite.h"

using namespace game;

PickupDrawer::PickupDrawer(const std::vector<Ammo>& ammo_pickups)
    : m_ammo_pickups(ammo_pickups)
{
    m_pickup_sprites[Pickups::AMMO] = mono::CreateSprite("res/sprites/ammo_pickup.sprite");
    m_pickup_sprites[Pickups::HEALTH] = mono::CreateSprite("res/sprites/health_pickup.sprite");
}

PickupDrawer::~PickupDrawer()
{ }

void PickupDrawer::doDraw(mono::IRenderer& renderer) const
{
    for(const Ammo& ammo : m_ammo_pickups)
        renderer.DrawSprite(*m_pickup_sprites[ammo.type], ammo.position);
}

math::Quad PickupDrawer::BoundingBox() const
{
    return math::InfQuad;
}
