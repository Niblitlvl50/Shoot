
#include "PlayerInteractionController.h"
#include "Events/PickupEvent.h"
#include "Player/Shuttle.h"

#include "EventHandler/EventHandler.h"

using namespace game;

PlayerInteractionController::PlayerInteractionController(Shuttle* player, mono::EventHandler& event_handler)
    : m_player(player)
    , m_event_handler(event_handler)
{
    using namespace std::placeholders;
    const std::function<bool (const game::PickupEvent&)>& pickup_func = std::bind(&PlayerInteractionController::OnPickup, this, _1);
    m_pickup_token = m_event_handler.AddListener(pickup_func);
}

PlayerInteractionController::~PlayerInteractionController()
{
    m_event_handler.RemoveListener(m_pickup_token);
}

bool PlayerInteractionController::OnPickup(const game::PickupEvent& event)
{
    if(event.entity_id != m_player->Id())
        return false;

    m_player->GiveAmmo(event.value);
    return true;
}
