
#include "PlayerGamepadController.h"
#include "PlayerLogic.h"
#include "Weapons/WeaponTypes.h"

#include "Events/TimeScaleEvent.h"
#include "Events/PauseEvent.h"
#include "EventHandler/EventHandler.h"
#include "System/System.h"

#include "Math/MathFunctions.h"
#include "Math/Vector.h"


#include "Events/EventFuncFwd.h"
#include "Events/ControllerEvent.h"

#include <algorithm>
#include <cmath>

using namespace game;

PlayerGamepadController::PlayerGamepadController(
    game::PlayerLogic* player_logic, mono::EventHandler* event_handler, const System::ControllerState& controller)
    : m_player_logic(player_logic)
    , m_event_handler(event_handler)
    , m_state(controller)
    , m_current_weapon_index(0)
    , m_pause(false)
{
    event::ControllerButtonDownFunc on_controller_down = [this](const event::ControllerButtonDownEvent& event) {
        if(event.button == System::ControllerButton::START)
        {
            m_pause = !m_pause;
            m_event_handler->DispatchEvent(event::PauseEvent(m_pause)); 
        }

        return mono::EventResult::PASS_ON;
    };
    m_controller_token = m_event_handler->AddListener(on_controller_down);
}

PlayerGamepadController::~PlayerGamepadController()
{
    m_event_handler->RemoveListener(m_controller_token);
}

void PlayerGamepadController::Update(const mono::UpdateContext& update_context)
{
    const bool fire = (m_state.right_trigger > 0.25f);
    if(fire)
        m_player_logic->Fire();
    else
        m_player_logic->StopFire();

    const bool a = System::IsButtonTriggered(m_last_state.button_state, m_state.button_state, System::ControllerButton::A);
    if(a)
        m_player_logic->TriggerInteraction();

    const bool b = System::IsButtonTriggered(m_last_state.button_state, m_state.button_state, System::ControllerButton::B);
    const bool b_changed = System::HasButtonChanged(m_last_state.button_state, m_state.button_state, System::ControllerButton::B);
    if(b)
        m_event_handler->DispatchEvent(event::TimeScaleEvent(0.1f));
    else if(b_changed)
        m_event_handler->DispatchEvent(event::TimeScaleEvent(1.0f));

    const bool reload =
        System::ButtonTriggeredAndChanged(m_last_state.button_state, m_state.button_state, System::ControllerButton::X);
    if(reload)
        m_player_logic->Reload(update_context.timestamp);

    const bool blink = System::IsButtonTriggered(m_last_state.button_state, m_state.button_state, System::ControllerButton::Y);
    if(blink)
        m_player_logic->Blink(math::Vector(m_state.left_x, m_state.left_y));

    const bool left_shoulder = System::IsButtonTriggered(m_last_state.button_state, m_state.button_state, System::ControllerButton::LEFT_SHOULDER);
    const bool right_shoulder = System::IsButtonTriggered(m_last_state.button_state, m_state.button_state, System::ControllerButton::RIGHT_SHOULDER);
    if(left_shoulder || right_shoulder)
    {
        left_shoulder ? --m_current_weapon_index : ++m_current_weapon_index;

        const std::vector<WeaponSetup> weapon_list = game::GetWeaponList();
        m_current_weapon_index = std::clamp(m_current_weapon_index, 0, (int)weapon_list.size());
        m_player_logic->SelectWeapon(weapon_list[m_current_weapon_index]);
    }

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
