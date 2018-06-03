
#include "Ammo.h"
#include "AIKnowledge.h"
#include "Events/PickupEvent.h"

#include "Math/Vector.h"
#include "EventHandler/EventHandler.h"


void game::CheckPlayerPickups(std::vector<Ammo>& pickups, mono::EventHandler& event_handler)
{
    if(!g_player_one.is_active)
        return;
    
    const math::Vector& player_one_position = g_player_one.position;
    std::vector<Ammo> pickups_to_dispatch;

    const auto check_for_pickup = [&player_one_position, &pickups_to_dispatch](const Ammo& pickup) {
        const float distance = math::Length(pickup.position - player_one_position);
        if(distance < 1.0f)
        {
            pickups_to_dispatch.push_back(pickup);
            return true;
        }

        return false;
    };

    const auto it = std::remove_if(pickups.begin(), pickups.end(), check_for_pickup);
    pickups.erase(it, pickups.end());

    for(const auto& ammo : pickups_to_dispatch)
        event_handler.DispatchEvent(game::PickupEvent(g_player_one.entity_id, ammo.value));
}
