
#include "ShuttleGamepadController.h"
#include "Shuttle.h"
#include "Weapons/WeaponTypes.h"

#include "Events/TimeScaleEvent.h"
#include "Events/QuitEvent.h"
#include "EventHandler/EventHandler.h"
#include "System/System.h"
#include "Math/MathFunctions.h"

#define IS_TRIGGERED(variable) (!m_last_state.variable && m_state.variable)
#define HAS_CHANGED(variable) (m_last_state.variable != m_state.variable)

using namespace game;

ShuttleGamepadController::ShuttleGamepadController(
    game::Shuttle* shuttle, mono::EventHandler& event_handler, const System::ControllerState& controller)
    : m_shuttle(shuttle),
      m_event_handler(event_handler),
      m_state(controller)
{ }

void ShuttleGamepadController::Update(unsigned int delta)
{
    const bool fire = (m_state.a || m_state.right_trigger > 0.25f);
    if(fire)
        m_shuttle->Fire();
    else
        m_shuttle->StopFire();

    const bool reload = IS_TRIGGERED(x) && HAS_CHANGED(x);
    if(reload)
        m_shuttle->Reload();

    const bool left_shoulder = IS_TRIGGERED(left_shoulder);
    const bool right_shoulder = IS_TRIGGERED(right_shoulder);
    if(left_shoulder)
        m_current_weapon_index = std::max(0, --m_current_weapon_index);
    else if(right_shoulder)
        m_current_weapon_index = std::min(N_WEAPON_TYPES, ++m_current_weapon_index);
        
    if(left_shoulder || right_shoulder)
        m_shuttle->SelectWeapon(static_cast<WeaponType>(m_current_weapon_index));
    
    const math::Vector force(m_state.left_x, m_state.left_y);
    m_shuttle->ApplyImpulse(force * 4);

    if(std::fabs(m_state.right_x) > 0.1f || std::fabs(m_state.right_y) > 0.1f)
    {
        const math::Vector direction(m_state.right_x, m_state.right_y);
        const float rotation = math::AngleBetweenPoints(math::zeroVec, direction) - math::PI_2();
        m_shuttle->SetRotation(rotation);
    }
    
    const bool leftBoosterOn = (force.x > 0.0f);
    const bool rightBoosterOn = (force.x < 0.0f);
    const bool mainBoosterOn = (force.y > 0.0f);
    
    //m_shuttle->SetBoosterThrusting(game::BoosterPosition::LEFT, leftBoosterOn);
    //m_shuttle->SetBoosterThrusting(game::BoosterPosition::RIGHT, rightBoosterOn);
    m_shuttle->SetBoosterThrusting(game::BoosterPosition::MAIN, mainBoosterOn || leftBoosterOn || rightBoosterOn);
    
    const bool b = IS_TRIGGERED(b);
    const bool b_changed = HAS_CHANGED(b);
    if(b)
        m_event_handler.DispatchEvent(event::TimeScaleEvent(0.5f));
    else if(b_changed)
        m_event_handler.DispatchEvent(event::TimeScaleEvent(1.0f));

    const bool quit = IS_TRIGGERED(y);
    if(quit)
        m_event_handler.DispatchEvent(event::QuitEvent());

    m_last_state = m_state;
}
