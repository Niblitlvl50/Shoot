
#include "PlayerKeybordController.h"
#include "Player/PlayerLogic.h"
#include "Player/PlayerInfo.h"
#include "Math/MathFunctions.h"

#include "Input/InputSystem.h"

using namespace game;

PlayerKeyboardController::PlayerKeyboardController(PlayerLogic* player_logic)
    : m_player_logic(player_logic)
    , m_left(false)
    , m_right(false)
    , m_up(false)
    , m_down(false)
    , m_fire(false)
    , m_trigger_reload(false)
    , m_trigger_action(false)
    , m_trigger_pickup_drop(false)
    , m_cycle_weapon(false)
    , m_update_aiming(false)
    , m_trigger_respawn(false)
{
}

void PlayerKeyboardController::Update(const mono::UpdateContext& update_context)
{
    if(m_player_logic->m_player_info->player_state == game::PlayerState::DEAD)
    {
        if(m_trigger_respawn)
        {
            m_player_logic->RespawnPlayer();
            m_trigger_respawn = false;
        }

        return;
    }

    // Update Movement
    math::Vector move_vector;

    if(m_left)
        move_vector.x -= 1.0f;
    if(m_right)
        move_vector.x += 1.0f;

    if(m_up)
        move_vector.y += 1.0f;
    if(m_down)
        move_vector.y -= 1.0f;

    /*
    if(System::IsKeyDown(Keycode::A))
        move_vector.x -= 1.0f;
    if(System::IsKeyDown(Keycode::D))
        move_vector.x += 1.0f;

    if(System::IsKeyDown(Keycode::W))
        move_vector.y += 1.0f;
    if(System::IsKeyDown(Keycode::S))
        move_vector.y -= 1.0f;
    */

    m_player_logic->MoveInDirection(math::Normalized(move_vector));

    // Update Aiming
    if(m_update_aiming)
    {
        const math::Vector position = m_player_logic->m_player_info->position;
        const math::Vector normalized_delta = math::Normalized(m_aim_position - position);
        const float aim_direction = math::AngleFromVector(normalized_delta);

        m_player_logic->SetAimDirection(aim_direction);
        m_player_logic->SetAimScreenPosition(m_aim_screen_position);

        m_update_aiming = false;
    }

    // Update Fire
    if(m_fire || System::IsKeyDown(Keycode::SPACE))
    {
        //const bool holding_pickup = m_player_logic->HoldingPickup();
        //if(holding_pickup)
        //    m_player_logic->ThrowAction();
        //else
            m_player_logic->Fire(update_context.timestamp);
    }
    else
    {
        m_player_logic->StopFire();
    }

    if(m_trigger_reload)
    {
        m_player_logic->Reload(update_context.timestamp);
        m_trigger_reload = false;
    }

    if(m_trigger_action)
    {
        m_player_logic->Blink(move_vector);
        m_trigger_action = false;
    }

    if(m_cycle_weapon)
    {
        m_player_logic->CycleWeapon();
        m_cycle_weapon = false;
    }

    if(m_trigger_pickup_drop)
    {
        m_player_logic->PickupDrop();
        m_trigger_pickup_drop = false;
    }
}

mono::InputResult PlayerKeyboardController::KeyDown(const event::KeyDownEvent& event)
{
    switch(event.key)
    {
    case Keycode::A:
        m_left = true;
        break;
    case Keycode::D:
        m_right = true;
        break;
    case Keycode::W:
        m_up = true;
        break;
    case Keycode::S:
        m_down = true;
        break;

    default:
        break;
    }

    return mono::InputResult::Handled;
}

mono::InputResult PlayerKeyboardController::KeyUp(const event::KeyUpEvent& event)
{
    switch(event.key)
    {
    case Keycode::A:
        m_left = false;
        break;
    case Keycode::D:
        m_right = false;
        break;
    case Keycode::W:
        m_up = false;
        break;
    case Keycode::S:
        m_down = false;
        break;

    case Keycode::R:
        m_trigger_reload = true;
        break;
    case Keycode::F:
        m_trigger_pickup_drop = true;
        break;
    case Keycode::TAB:
        m_cycle_weapon = true;
        break;
    case Keycode::ESCAPE:
    {
        m_player_logic->TogglePauseGame();
        break;
    }
    case Keycode::ENTER:
        m_trigger_respawn = true;
        break;

    default:
        break;
    }

    return mono::InputResult::Handled;
}

mono::InputResult PlayerKeyboardController::Move(const event::MouseMotionEvent& event)
{
    m_aim_position = {event.world_x, event.world_y};
    m_aim_screen_position = math::Vector(event.screen_x, event.screen_y);
    m_update_aiming = true;

    return mono::InputResult::Pass;
}

mono::InputResult PlayerKeyboardController::ButtonDown(const event::MouseDownEvent& event)
{
    if(event.key == MouseButton::LEFT)
        m_fire = true;

    return mono::InputResult::Handled;
}

mono::InputResult PlayerKeyboardController::ButtonUp(const event::MouseUpEvent& event)
{
    if(event.key == MouseButton::LEFT)
        m_fire = false;
    else if(event.key == MouseButton::RIGHT)
        m_trigger_action = true;

    return mono::InputResult::Handled;
}
