
#include "TrainKeyboardController.h"
#include "Player/TrainLogic.h"
#include "Player/PlayerInfo.h"

using namespace game;

TrainKeyboardController::TrainKeyboardController(TrainLogic* train_logic)
    : m_train_logic(train_logic)
    , m_raise_throttle(false)
    , m_lower_throttle(false)
    , m_load_held(false)
    , m_toggle_direction(false)
    , m_honk(false)
    , m_decouple(false)
    , m_trigger_pickup_drop(false)
    , m_trigger_respawn(false)
{ }

void TrainKeyboardController::Update(const mono::UpdateContext& update_context)
{
    if(m_train_logic->m_player_info->player_state == game::PlayerState::DEAD)
    {
        m_train_logic->SetLoadHeld(false);

        if(m_trigger_respawn)
            m_train_logic->RespawnPlayer();

        m_trigger_respawn = false;
        return;
    }

    if(m_toggle_direction)
    {
        m_train_logic->ToggleDirection();
        m_toggle_direction = false;
    }

    const float raise_input = m_raise_throttle ? 1.0f : 0.0f;
    const float lower_input = m_lower_throttle ? 1.0f : 0.0f;
    m_train_logic->AdjustThrottle(raise_input, lower_input, update_context.delta_s);
    m_train_logic->SetLoadHeld(m_load_held);

    if(m_honk)
    {
        m_train_logic->Honk();
        m_honk = false;
    }

    if(m_decouple)
    {
        m_train_logic->Decouple();
        m_decouple = false;
    }

    if(m_trigger_pickup_drop)
    {
        m_train_logic->PickupDrop();
        m_trigger_pickup_drop = false;
    }

    m_trigger_respawn = false;
}

mono::InputResult TrainKeyboardController::KeyDown(const event::KeyDownEvent& event)
{
    switch(event.key)
    {
    case Keycode::W:
    case Keycode::UP:
        m_raise_throttle = true;
        break;
    case Keycode::S:
    case Keycode::DOWN:
        m_lower_throttle = true;
        break;
    case Keycode::E:
        m_load_held = true;
        break;

    default:
        break;
    }

    return mono::InputResult::Handled;
}

mono::InputResult TrainKeyboardController::KeyUp(const event::KeyUpEvent& event)
{
    switch(event.key)
    {
    case Keycode::W:
    case Keycode::UP:
        m_raise_throttle = false;
        break;
    case Keycode::S:
    case Keycode::DOWN:
        m_lower_throttle = false;
        break;
    case Keycode::E:
        m_load_held = false;
        break;

    case Keycode::R:
        m_toggle_direction = true;
        break;
    case Keycode::SPACE:
    case Keycode::H:
        m_honk = true;
        break;
    case Keycode::C:
        m_decouple = true;
        break;
    case Keycode::F:
        m_trigger_pickup_drop = true;
        break;
    case Keycode::ESCAPE:
        m_train_logic->TogglePauseGame();
        break;
    case Keycode::ENTER:
        m_trigger_respawn = true;
        break;

    default:
        break;
    }

    return mono::InputResult::Handled;
}
