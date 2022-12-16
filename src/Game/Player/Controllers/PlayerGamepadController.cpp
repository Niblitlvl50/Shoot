
#include "PlayerGamepadController.h"
#include "Player/PlayerLogic.h"
#include "Player/PlayerInfo.h"

#include "System/System.h"
#include "Math/MathFunctions.h"
#include "Math/Vector.h"

#include <algorithm>
#include <cmath>

using namespace game;

PlayerGamepadController::PlayerGamepadController(
    game::PlayerLogic* player_logic,
    mono::InputSystem* input_system,
    const System::ControllerState& controller)
    : m_player_logic(player_logic)
    , m_input_system(input_system)
    , m_last_input_timestamp(0)
    , m_state(controller)
{
    m_input_context = m_input_system->CreateContext(1, mono::InputContextBehaviour::ConsumeIfHandled, "PlayerGamepadController");
    m_input_context->controller_input = this;
}

PlayerGamepadController::~PlayerGamepadController()
{
    m_input_system->ReleaseContext(m_input_context);
}

void PlayerGamepadController::Update(const mono::UpdateContext& update_context)
{
    if(m_player_logic->m_player_info->player_state == game::PlayerState::DEAD)
    {
        const bool respawn_pressed =
            System::ButtonTriggeredAndChanged(m_last_state.button_state, m_state.button_state, System::ControllerButton::FACE_BOTTOM);
        if(respawn_pressed)
            m_player_logic->RespawnPlayer();

        return;
    }

    const bool fire = (m_state.right_trigger > 0.25f);
    if(fire)
        m_player_logic->Fire(update_context.timestamp);
    else
        m_player_logic->StopFire();

    const bool do_shockwave = System::IsButtonTriggered(m_last_state.button_state, m_state.button_state, System::ControllerButton::FACE_BOTTOM);
    if(do_shockwave)
    {
        m_player_logic->Shockwave();
        //m_player_logic->TriggerInteraction();
    }

    const bool reload =
        System::ButtonTriggeredAndChanged(m_last_state.button_state, m_state.button_state, System::ControllerButton::FACE_LEFT);
    if(reload)
        m_player_logic->Reload(update_context.timestamp);

    const bool blink = System::IsButtonTriggered(m_last_state.button_state, m_state.button_state, System::ControllerButton::FACE_TOP);
    if(blink)
        m_player_logic->Blink(math::Vector(m_state.left_x, m_state.left_y));

    const bool left_shoulder = System::IsButtonTriggered(m_last_state.button_state, m_state.button_state, System::ControllerButton::LEFT_SHOULDER);
    if(left_shoulder)
        m_player_logic->SelectWeapon(PlayerLogic::WeaponSelection::Previous);

    const bool right_shoulder = System::IsButtonTriggered(m_last_state.button_state, m_state.button_state, System::ControllerButton::RIGHT_SHOULDER);
    if(right_shoulder)
        m_player_logic->SelectWeapon(PlayerLogic::WeaponSelection::Next);

    m_player_logic->MoveInDirection(math::Vector(m_state.left_x, m_state.left_y));

    if(std::fabs(m_state.right_x) > 0.1f || std::fabs(m_state.right_y) > 0.1f)
    {
        const float aim_direction = math::AngleFromVector(math::Vector(m_state.right_x, m_state.right_y));
        m_player_logic->SetAimDirection(aim_direction);
    }

    const bool left_triggered = System::IsButtonTriggered(m_last_state.button_state, m_state.button_state, System::ControllerButton::LEFT);
    const bool right_triggered = System::IsButtonTriggered(m_last_state.button_state, m_state.button_state, System::ControllerButton::RIGHT);
    const bool up_triggered = System::IsButtonTriggered(m_last_state.button_state, m_state.button_state, System::ControllerButton::UP);
    const bool down_triggered = System::IsButtonTriggered(m_last_state.button_state, m_state.button_state, System::ControllerButton::DOWN);

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

    m_last_state = m_state;
}

uint32_t PlayerGamepadController::GetLastInputTimestamp() const
{
    return m_last_input_timestamp;
}

mono::InputResult PlayerGamepadController::ButtonDown(const event::ControllerButtonDownEvent& event)
{
    if(event.controller_id == m_player_logic->m_player_info->controller_id)
    {
        if(event.button == System::ControllerButton::START || event.button == System::ControllerButton::TOUCHPAD)
            m_player_logic->TogglePauseGame();

        m_last_input_timestamp = event.timestamp;
    }

    return mono::InputResult::Pass;
}

mono::InputResult PlayerGamepadController::Axis(const event::ControllerAxisEvent& event)
{
    if(event.controller_id == m_player_logic->m_player_info->controller_id)
        m_last_input_timestamp = event.timestamp;
    return mono::InputResult::Pass;
}
