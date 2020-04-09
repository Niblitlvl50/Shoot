
#include "PlayerInteractionController.h"
#include "Events/PickupEvent.h"
#include "Player/PlayerLogic.h"

#include "Audio/AudioFactory.h"
#include "EventHandler/EventHandler.h"


namespace
{
    using PickupFunc = void (game::PlayerLogic::*)(int amount);

    constexpr PickupFunc pickup_func_table[] = {
        &game::PlayerLogic::GiveAmmo,
        &game::PlayerLogic::GiveHealth
    };
}

using namespace game;

PlayerInteractionController::PlayerInteractionController(PlayerLogic* shuttle_logic, mono::EventHandler& event_handler)
    : m_shuttle_logic(shuttle_logic)
    , m_event_handler(event_handler)
{
    using namespace std::placeholders;
    const std::function<mono::EventResult (const game::PickupEvent&)>& pickup_func = std::bind(&PlayerInteractionController::OnPickup, this, _1);
    m_pickup_token = m_event_handler.AddListener(pickup_func);

    m_pickup_sound = mono::AudioFactory::CreateSound("res/sound/item_pickup.wav", mono::SoundPlayback::ONCE, mono::SoundPosition::RELATIVE);
}

PlayerInteractionController::~PlayerInteractionController()
{
    m_event_handler.RemoveListener(m_pickup_token);
}

mono::EventResult PlayerInteractionController::OnPickup(const game::PickupEvent& event)
{
    if(event.entity_id != m_shuttle_logic->EntityId())
        return mono::EventResult::PASS_ON;

    const PickupFunc pickup_func = pickup_func_table[event.type];
    (m_shuttle_logic->*pickup_func)(event.value);
    m_pickup_sound->Play();

    return mono::EventResult::HANDLED;
}
