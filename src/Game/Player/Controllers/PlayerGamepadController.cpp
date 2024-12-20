
#include "PlayerGamepadController.h"
#include "Player/PlayerLogic.h"
#include "Player/PlayerInfo.h"

#include "System/System.h"
#include "Math/MathFunctions.h"
#include "Math/Vector.h"

#include <algorithm>
#include <cmath>

using namespace game;

PlayerGamepadController::PlayerGamepadController(game::PlayerLogic* player_logic)
    : m_player_logic(player_logic)
{ }

void PlayerGamepadController::Update(const mono::UpdateContext& update_context)
{
    if(m_player_logic->m_player_info->player_state == game::PlayerState::DEAD)
    {
        const bool respawn_pressed =
            System::ButtonTriggeredAndChanged(m_last_state.button_state, m_current_state.button_state, System::ControllerButton::FACE_BOTTOM);
        if(respawn_pressed)
            m_player_logic->RespawnPlayer();

        return;
    }

    const bool fire = (m_current_state.right_trigger > 0.25f);
    if(fire)
        m_player_logic->Fire(update_context.timestamp);
    else
        m_player_logic->StopFire();

    const bool sprint = System::IsButtonDown(m_current_state.button_state, System::ControllerButton::FACE_LEFT);
    if(sprint)
        m_player_logic->Sprint();
    else
        m_player_logic->StopSprint();

    const bool reload =
        System::ButtonTriggeredAndChanged(m_last_state.button_state, m_current_state.button_state, System::ControllerButton::FACE_RIGHT);
    if(reload)
        m_player_logic->Reload(update_context.timestamp);

    const bool cycle_weapon = System::IsButtonTriggered(m_last_state.button_state, m_current_state.button_state, System::ControllerButton::FACE_TOP);
    if(cycle_weapon)
        m_player_logic->CycleWeapon();

    const bool blink = System::IsButtonTriggered(m_last_state.button_state, m_current_state.button_state, System::ControllerButton::FACE_BOTTOM);
    if(blink)
        m_player_logic->Blink(math::Vector(m_current_state.left_x, m_current_state.left_y));

    const bool right_shoulder = System::IsButtonDown(m_current_state.button_state, System::ControllerButton::RIGHT_SHOULDER);
    if(right_shoulder)
        m_player_logic->TriggerHookshot();
    else
        m_player_logic->ReleaseHookshot();

    m_player_logic->MoveInDirection(math::Vector(m_current_state.left_x, m_current_state.left_y));

    const bool x_axis_changed = (std::fabs(m_current_state.right_x) > 0.1f);
    const bool y_axis_changed = (std::fabs(m_current_state.right_y) > 0.1f);
    if(x_axis_changed || y_axis_changed)
    {
        const math::Vector aim_vector = {
            x_axis_changed ? m_current_state.right_x : 0.0f,
            y_axis_changed ? m_current_state.right_y : 0.0f
        };
        const float aim_direction = math::AngleFromVector(aim_vector);
        m_player_logic->SetAimDirection(aim_direction);
    }

    const bool left_triggered = System::IsButtonTriggered(m_last_state.button_state, m_current_state.button_state, System::ControllerButton::LEFT);
    const bool right_triggered = System::IsButtonTriggered(m_last_state.button_state, m_current_state.button_state, System::ControllerButton::RIGHT);
    const bool up_triggered = System::IsButtonTriggered(m_last_state.button_state, m_current_state.button_state, System::ControllerButton::UP);
    const bool down_triggered = System::IsButtonTriggered(m_last_state.button_state, m_current_state.button_state, System::ControllerButton::DOWN);

    if(left_triggered || right_triggered)
    {
        ItemSlotIndex slot_index;

        if(left_triggered)
            slot_index = ItemSlotIndex::LEFT;
        else
            slot_index = ItemSlotIndex::RIGHT;

        m_player_logic->UseItemSlot(slot_index);
    }

    if(up_triggered)
        m_player_logic->ThrowAction();

    if(down_triggered)
        m_player_logic->PickupDrop();
}

mono::InputResult PlayerGamepadController::ButtonDown(const event::ControllerButtonDownEvent& event)
{
    if(event.controller_id == m_player_logic->m_player_info->controller_id)
    {
        if(event.button == System::ControllerButton::START || event.button == System::ControllerButton::TOUCHPAD)
            m_player_logic->TogglePauseGame();
    }

    return mono::InputResult::Pass;
}

mono::InputResult PlayerGamepadController::Axis(const event::ControllerAxisEvent& event)
{
    return mono::InputResult::Pass;
}

mono::InputResult PlayerGamepadController::UpdatedControllerState(const System::ControllerState& updated_state)
{
    m_last_state = m_current_state;
    m_current_state = updated_state;
    return mono::InputResult::Pass;
}
