
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

#define IS_TRIGGERED(variable) (!m_last_state.variable && m_state.variable)
#define HAS_CHANGED(variable) (m_last_state.variable != m_state.variable)

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

    const bool secondary_fire = IS_TRIGGERED(y);
    if(secondary_fire)
        m_shuttle_logic->SecondaryFire();

    const bool reload = IS_TRIGGERED(x) && HAS_CHANGED(x);
    if(reload)
        m_shuttle_logic->Reload(update_context.timestamp);

    const bool left_shoulder = IS_TRIGGERED(left_shoulder);
    const bool right_shoulder = IS_TRIGGERED(right_shoulder);
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
    
    const bool b = IS_TRIGGERED(b);
    const bool b_changed = HAS_CHANGED(b);
    if(b)
        m_event_handler.DispatchEvent(event::TimeScaleEvent(0.5f));
    else if(b_changed)
        m_event_handler.DispatchEvent(event::TimeScaleEvent(1.0f));

    const bool left_triggered = IS_TRIGGERED(left);
    const bool right_triggered = IS_TRIGGERED(right);
    const bool up_triggered = IS_TRIGGERED(up);
    const bool down_triggered = IS_TRIGGERED(down);

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
