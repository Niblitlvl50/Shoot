
#include "TrainGamepadController.h"
#include "Player/TrainLogic.h"
#include "Player/PlayerInfo.h"

#include "System/System.h"
#include "Math/MathFunctions.h"
#include "Math/Vector.h"

#include <cmath>

using namespace game;

TrainGamepadController::TrainGamepadController(game::TrainLogic* train_logic)
    : m_train_logic(train_logic)
{ }

void TrainGamepadController::Update(const mono::UpdateContext& update_context)
{
    if(m_train_logic->m_player_info->player_state == game::PlayerState::DEAD)
    {
        const bool respawn_pressed =
            System::ButtonTriggeredAndChanged(m_last_state.button_state, m_current_state.button_state, System::ControllerButton::FACE_BOTTOM);
        if(respawn_pressed)
            m_train_logic->RespawnPlayer();

        return;
    }

    const float combined_throttle = -m_current_state.left_trigger + m_current_state.right_trigger;
    m_train_logic->SetThrottle(combined_throttle);

    const bool right_shoulder = System::IsButtonDown(m_current_state.button_state, System::ControllerButton::RIGHT_SHOULDER);
    if(right_shoulder)
        m_train_logic->TriggerHookshot();
    else
        m_train_logic->ReleaseHookshot();

    const bool x_axis_changed = (std::fabs(m_current_state.right_x) > 0.1f);
    const bool y_axis_changed = (std::fabs(m_current_state.right_y) > 0.1f);
    if(x_axis_changed || y_axis_changed)
    {
        const math::Vector aim_vector = {
            x_axis_changed ? m_current_state.right_x : 0.0f,
            y_axis_changed ? m_current_state.right_y : 0.0f
        };
        const float aim_direction = math::AngleFromVector(aim_vector);
        //m_train_logic->SetAimDirection(aim_direction);
    }

    const bool up_triggered = System::IsButtonTriggered(m_last_state.button_state, m_current_state.button_state, System::ControllerButton::UP);
    const bool down_triggered = System::IsButtonTriggered(m_last_state.button_state, m_current_state.button_state, System::ControllerButton::DOWN);

    if(up_triggered)
        m_train_logic->ThrowAction();

    if(down_triggered)
        m_train_logic->PickupDrop();

    const bool honk_triggered = System::IsButtonTriggered(m_last_state.button_state, m_current_state.button_state, System::ControllerButton::FACE_TOP);
    if(honk_triggered)
        m_train_logic->Honk();
}

mono::InputResult TrainGamepadController::ButtonDown(const event::ControllerButtonDownEvent& event)
{
    if(event.controller_id == m_train_logic->m_player_info->controller_id)
    {
        if(event.button == System::ControllerButton::START || event.button == System::ControllerButton::TOUCHPAD)
            m_train_logic->TogglePauseGame();
    }

    return mono::InputResult::Pass;
}

mono::InputResult TrainGamepadController::Axis(const event::ControllerAxisEvent& event)
{
    return mono::InputResult::Pass;
}

mono::InputResult TrainGamepadController::UpdatedControllerState(const System::ControllerState& updated_state)
{
    m_last_state = m_current_state;
    m_current_state = updated_state;
    return mono::InputResult::Pass;
}
