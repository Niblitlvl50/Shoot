
#include "PickupDrawer.h"
#include "Math/Quad.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"
#include "Pickups/Ammo.h"

using namespace game;

PickupDrawer::PickupDrawer(const std::vector<Ammo>& ammo_pickups)
    : m_ammo_pickups(ammo_pickups)
{ }

void PickupDrawer::doDraw(mono::IRenderer& renderer) const
{
    std::vector<math::Vector> ammo_pickups;
    ammo_pickups.resize(m_ammo_pickups.size());

    const auto ammo_to_point = [](const Ammo& ammo) {
        return ammo.position;
    };

    std::transform(m_ammo_pickups.begin(), m_ammo_pickups.end(), ammo_pickups.begin(), ammo_to_point);

    constexpr mono::Color::RGBA color(0.0f, 1.0f, 0.5f);
    renderer.DrawPoints(ammo_pickups, color, 5.0f);
}

math::Quad PickupDrawer::BoundingBox() const
{
    return math::InfQuad;
}
