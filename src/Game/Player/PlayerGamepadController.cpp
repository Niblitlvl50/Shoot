
#include "PlayerGamepadController.h"
#include "PlayerLogic.h"
#include "Weapons/WeaponTypes.h"

#include "Events/TimeScaleEvent.h"
#include "Events/QuitEvent.h"
#include "EventHandler/EventHandler.h"
#include "System/System.h"

#include "Math/MathFunctions.h"
#include "Math/Vector.h"

#include <algorithm>
#include <cmath>

#define IS_TRIGGERED(variable) (!m_last_state.variable && m_state.variable)
#define HAS_CHANGED(variable) (m_last_state.variable != m_state.variable)

using namespace game;

namespace
{
    template<class T>
    constexpr const T& clamp(const T& value, const T& min, const T& max)
    {
        return std::max(min, std::min(value, max));
    }
}


PlayerGamepadController::PlayerGamepadController(
    game::PlayerLogic* shuttle_logic, mono::EventHandler& event_handler, const System::ControllerState& controller)
    : m_shuttle_logic(shuttle_logic),
      m_event_handler(event_handler),
      m_state(controller)
{ }

void PlayerGamepadController::Update(uint32_t delta_ms)
{
    const bool fire = (m_state.a || m_state.right_trigger > 0.25f);
    if(fire)
        m_shuttle_logic->Fire();
    else
        m_shuttle_logic->StopFire();

    const bool reload = IS_TRIGGERED(x) && HAS_CHANGED(x);
    if(reload)
        m_shuttle_logic->Reload();

    const bool left_shoulder = IS_TRIGGERED(left_shoulder);
    const bool right_shoulder = IS_TRIGGERED(right_shoulder);
    if(left_shoulder)
        --m_current_weapon_index;
    else if(right_shoulder)
        ++m_current_weapon_index;

    m_current_weapon_index = clamp(m_current_weapon_index, 0, N_WEAPON_TYPES);
        
    if(left_shoulder || right_shoulder)
        m_shuttle_logic->SelectWeapon(static_cast<WeaponType>(m_current_weapon_index));
    
    const math::Vector force(m_state.left_x, m_state.left_y);
    m_shuttle_logic->ApplyImpulse(force * 2);

    if(std::fabs(m_state.right_x) > 0.1f || std::fabs(m_state.right_y) > 0.1f)
    {
        const math::Vector direction(m_state.right_x, m_state.right_y);
        const float rotation = math::AngleBetweenPoints(math::ZeroVec, direction) - math::PI_2();
        m_shuttle_logic->SetRotation(rotation);
    }
    
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

    const bool quit = IS_TRIGGERED(y);
    if(quit)
        m_event_handler.DispatchEvent(event::QuitEvent());

    m_last_state = m_state;
}
