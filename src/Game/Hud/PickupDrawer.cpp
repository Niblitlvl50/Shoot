
#include "PickupDrawer.h"
#include "Math/Quad.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Sprite/SpriteFactory.h"
#include "Rendering/Sprite/ISprite.h"
#include "Pickups/Ammo.h"

using namespace game;

PickupDrawer::PickupDrawer(const std::vector<Ammo>& ammo_pickups)
    : m_ammo_pickups(ammo_pickups)
{
    m_ammo_sprite = mono::CreateSprite("res/sprites/ammo_pickup.sprite");
}

void PickupDrawer::doDraw(mono::IRenderer& renderer) const
{
    for(const Ammo& ammo : m_ammo_pickups)
        renderer.DrawSprite(*m_ammo_sprite, ammo.position);
}

math::Quad PickupDrawer::BoundingBox() const
{
    return math::InfQuad;
}
