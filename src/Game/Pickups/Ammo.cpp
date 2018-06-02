
#include "Ammo.h"
#include "Math/Vector.h"
#include "Events/PickupEvent.h"
#include "EventHandler/EventHandler.h"

#include "AIKnowledge.h"

void game::CheckPlayerPickups(std::vector<Ammo>& pickups, mono::EventHandler& event_handler)
{
    if(!g_player_one.is_active)
        return;
    
    const math::Vector& player_one_position = g_player_one.position;

    const auto check_for_pickup = [&player_one_position](const Ammo& pickup) {
        const float distance = math::Length(pickup.position - player_one_position);
        return distance < 1.0f;
    };

    auto find_it = std::remove_if(pickups.begin(), pickups.end(), check_for_pickup);
    for(auto it = find_it; it != pickups.end(); ++it)
    {
        event_handler.DispatchEvent(game::PickupEvent(g_player_one.entity_id, it->value));
    }

    pickups.erase(find_it, pickups.end());
}
