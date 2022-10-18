
#include "PlayerKeybordController.h"
#include "PlayerLogic.h"
#include "PlayerInfo.h"

#include "EventHandler/EventHandler.h"
#include "Events/EventFuncFwd.h"
#include "Events/MouseEvent.h"
#include "Events/KeyEvent.h"

using namespace game;

PlayerKeyboardController::PlayerKeyboardController(PlayerLogic* player_logic, mono::EventHandler* event_handler)
    : m_player_logic(player_logic)
    , m_event_handler(event_handler)
    , m_pause(false)
    , m_fire(false)
    , m_trigger_reload(false)
    , m_trigger_action(false)
    , m_trigger_previous_weapon(false)
    , m_trigger_next_weapon(false)
{
    using namespace std::placeholders;

    const event::MouseDownEventFunc mouse_down_func = std::bind(&PlayerKeyboardController::OnMouseDown, this, _1);
    const event::MouseUpEventFunc mouse_up_func = std::bind(&PlayerKeyboardController::OnMouseUp, this, _1);
    const event::MouseMotionEventFunc mouse_motion_func = std::bind(&PlayerKeyboardController::OnMouseMotion, this, _1);
    const event::KeyDownEventFunc key_down_func = std::bind(&PlayerKeyboardController::OnKeyDown, this, _1);
    const event::KeyUpEventFunc key_up_func = std::bind(&PlayerKeyboardController::OnKeyUp, this, _1);

    m_mouse_down_token = m_event_handler->AddListener(mouse_down_func);
    m_mouse_up_token = m_event_handler->AddListener(mouse_up_func);
    m_mouse_motion_token = m_event_handler->AddListener(mouse_motion_func);
    m_key_down_token = m_event_handler->AddListener(key_down_func);
    m_key_up_token = m_event_handler->AddListener(key_up_func);
}

PlayerKeyboardController::~PlayerKeyboardController()
{
    m_event_handler->RemoveListener(m_mouse_down_token);
    m_event_handler->RemoveListener(m_mouse_up_token);
    m_event_handler->RemoveListener(m_mouse_motion_token);
    m_event_handler->RemoveListener(m_key_down_token);
    m_event_handler->RemoveListener(m_key_up_token);
}

void PlayerKeyboardController::Update(const mono::UpdateContext& update_context)
{
    // Update Movement
    math::Vector move_vector;

    if(System::IsKeyDown(Keycode::A))
        move_vector.x -= 1.0f;
    if(System::IsKeyDown(Keycode::D))
        move_vector.x += 1.0f;

    if(System::IsKeyDown(Keycode::W))
        move_vector.y += 1.0f;
    if(System::IsKeyDown(Keycode::S))
        move_vector.y -= 1.0f;

    m_player_logic->MoveInDirection(math::Normalized(move_vector));

    // Update Aiming
    const math::Vector position = m_player_logic->m_player_info->position;
    const math::Vector normalized_delta = math::Normalized(m_aim_position - position);
    const float aim_direction = math::AngleFromVector(normalized_delta);

    m_player_logic->SetAimDirection(aim_direction);

    // Update Fire
    if(m_fire || System::IsKeyDown(Keycode::SPACE))
        m_player_logic->Fire();
    else
        m_player_logic->StopFire();

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

    if(m_trigger_previous_weapon)
    {
        m_player_logic->SelectWeapon(PlayerLogic::WeaponSelection::Previous);
        m_trigger_previous_weapon = false;
    }

    if(m_trigger_next_weapon)
    {
        m_player_logic->SelectWeapon(PlayerLogic::WeaponSelection::Next);
        m_trigger_next_weapon = false;
    }
}

mono::EventResult PlayerKeyboardController::OnMouseDown(const event::MouseDownEvent& event)
{
    if(event.key == MouseButton::LEFT)
        m_fire = true;

    return mono::EventResult::HANDLED;
}

mono::EventResult PlayerKeyboardController::OnMouseUp(const event::MouseUpEvent& event)
{
    if(event.key == MouseButton::LEFT)
        m_fire = false;

    return mono::EventResult::HANDLED;
}

mono::EventResult PlayerKeyboardController::OnMouseMotion(const event::MouseMotionEvent& event)
{
    m_aim_position = {event.world_x, event.world_y};
    return mono::EventResult::HANDLED;
}

mono::EventResult PlayerKeyboardController::OnKeyDown(const event::KeyDownEvent& event)
{
    switch(event.key)
    {
    default:
        break;
    }

    return mono::EventResult::HANDLED;
}

mono::EventResult PlayerKeyboardController::OnKeyUp(const event::KeyUpEvent& event)
{
    switch(event.key)
    {
    case Keycode::R:
        m_trigger_reload = true;
        break;
    case Keycode::F:
        m_trigger_action = true;
        break;
    case Keycode::ONE:
        m_trigger_previous_weapon = true;
        break;
    case Keycode::TWO:
        m_trigger_next_weapon = true;
        break;

    default:
        break;
    }

    return mono::EventResult::HANDLED;
}
