
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
    game::PlayerLogic* shuttle_logic, mono::EventHandler& event_handler, const System::ControllerState& controller)
    : m_shuttle_logic(shuttle_logic),
      m_event_handler(event_handler),
      m_state(controller)
{ }

void PlayerGamepadController::Update(const mono::UpdateContext& update_context)
{
    const bool fire = (m_state.right_trigger > 0.25f);
    if(fire)
        m_shuttle_logic->Fire();
    else
        m_shuttle_logic->StopFire();

    const bool secondary_fire = System::IsButtonTriggered(m_last_state.button_state, m_state.button_state, System::ControllerButton::Y);
    if(secondary_fire)
        m_shuttle_logic->SecondaryFire();

    const bool reload =
        System::ButtonTriggeredAndChanged(m_last_state.button_state, m_state.button_state, System::ControllerButton::X);
    if(reload)
        m_shuttle_logic->Reload(update_context.timestamp);

    const bool left_shoulder = System::IsButtonTriggered(m_last_state.button_state, m_state.button_state, System::ControllerButton::LEFT_SHOULDER);
    const bool right_shoulder = System::IsButtonTriggered(m_last_state.button_state, m_state.button_state, System::ControllerButton::RIGHT_SHOULDER);
    if(left_shoulder)
        --m_current_weapon_index;
    else if(right_shoulder)
        ++m_current_weapon_index;

    m_current_weapon_index = std::clamp(m_current_weapon_index, 0, N_WEAPON_TYPES);
        
    if(left_shoulder || right_shoulder)
        m_shuttle_logic->SelectWeapon(static_cast<WeaponType>(m_current_weapon_index));
    
    const math::Vector force(m_state.left_x, m_state.left_y);
    const float length_squared = math::LengthSquared(force);
    if(length_squared <= FLT_EPSILON)
        m_shuttle_logic->ResetForces();
    else
        //m_shuttle_logic->ApplyForce(force * 40.0f);
        m_shuttle_logic->ApplyImpulse(force * 10.0f);
        //m_shuttle_logic->SetVelocity(force * 4.0f);

    //if(std::fabs(m_state.right_x) > 0.1f || std::fabs(m_state.right_y) > 0.1f)
    //{
    //    const math::Vector direction(m_state.right_x, m_state.right_y);
    //    const float rotation = math::NormalizeAngle(math::AngleBetweenPoints(math::ZeroVec, direction) - math::PI_2());
    //    m_shuttle_logic->SetRotation(rotation);
    //}
    
    PlayerAnimation animation = PlayerAnimation::IDLE;
    if(force.x > 0.0f)
        animation = PlayerAnimation::WALK_RIGHT;
    else if(force.x < 0.0f)
        animation = PlayerAnimation::WALK_LEFT;

    m_shuttle_logic->SetAnimation(animation);
    
    const bool b = System::IsButtonTriggered(m_last_state.button_state, m_state.button_state, System::ControllerButton::B);
    const bool b_changed = System::HasButtonChanged(m_last_state.button_state, m_state.button_state, System::ControllerButton::B);
    if(b)
        m_event_handler.DispatchEvent(event::TimeScaleEvent(0.1f));
    else if(b_changed)
        m_event_handler.DispatchEvent(event::TimeScaleEvent(1.0f));

    const bool left_triggered = System::IsButtonTriggered(m_last_state.button_state, m_state.button_state, System::ControllerButton::LEFT);
    const bool right_triggered = System::IsButtonTriggered(m_last_state.button_state, m_state.button_state, System::ControllerButton::RIGHT);
    const bool up_triggered = System::IsButtonTriggered(m_last_state.button_state, m_state.button_state, System::ControllerButton::UP);
    const bool down_triggered = System::IsButtonTriggered(m_last_state.button_state, m_state.button_state, System::ControllerButton::DOWN);

    if(left_triggered || right_triggered || up_triggered || down_triggered)
    {
        BlinkDirection direction;

        if(left_triggered)
            direction = BlinkDirection::LEFT;
        else if(right_triggered)
            direction = BlinkDirection::RIGHT;
        else if(up_triggered)
            direction = BlinkDirection::UP;
        else
            direction = BlinkDirection::DOWN;

        m_shuttle_logic->Blink(direction);
    }

    m_last_state = m_state;
}
