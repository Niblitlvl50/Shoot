
#include "PlayerGamepadController.h"
#include "PlayerLogic.h"
#include "Weapons/WeaponTypes.h"

#include "Events/TimeScaleEvent.h"
#include "EventHandler/EventHandler.h"
#include "System/System.h"

#include "Math/MathFunctions.h"
#include "Math/Vector.h"

#include <algorithm>
#include <cmath>

using namespace game;

PlayerGamepadController::PlayerGamepadController(
    game::PlayerLogic* player_logic, mono::EventHandler* event_handler, const System::ControllerState& controller)
    : m_player_logic(player_logic)
    , m_event_handler(event_handler)
    , m_state(controller)
{ }

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

    const math::Vector force(m_state.left_x, m_state.left_y);
    m_player_logic->MoveInDirection(math::Vector(m_state.left_x, m_state.left_y));

    if(std::fabs(m_state.right_x) > 0.1f || std::fabs(m_state.right_y) > 0.1f)
    {
        const math::Vector direction(m_state.right_x, m_state.right_y);
        const float aim_direction = math::NormalizeAngle(math::AngleBetweenPoints(math::ZeroVec, direction) - math::PI_2());
        m_player_logic->SetAimDirection(aim_direction);
    }

    const bool left_triggered = System::IsButtonTriggered(m_last_state.button_state, m_state.button_state, System::ControllerButton::LEFT);
    const bool right_triggered = System::IsButtonTriggered(m_last_state.button_state, m_state.button_state, System::ControllerButton::RIGHT);
    const bool up_triggered = System::IsButtonTriggered(m_last_state.button_state, m_state.button_state, System::ControllerButton::UP);
    const bool down_triggered = System::IsButtonTriggered(m_last_state.button_state, m_state.button_state, System::ControllerButton::DOWN);

    if(left_triggered || right_triggered || up_triggered || down_triggered)
    {
        ItemSlotIndex slot_index;

        if(left_triggered)
            slot_index = ItemSlotIndex::LEFT;
        else if(right_triggered)
            slot_index = ItemSlotIndex::RIGHT;
        else if(up_triggered)
            slot_index = ItemSlotIndex::UP;
        else
            slot_index = ItemSlotIndex::DOWN;

        m_player_logic->UseItemSlot(slot_index);
    }

    m_last_state = m_state;
}
