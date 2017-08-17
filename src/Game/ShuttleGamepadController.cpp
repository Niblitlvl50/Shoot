
#include "ShuttleGamepadController.h"
#include "Shuttle.h"
#include "Weapons/WeaponTypes.h"

#include "EventHandler/EventHandler.h"
#include "Events/ControllerEvent.h"
#include "Events/TimeScaleEvent.h"

#include "System/System.h"


ShuttleGamepadController::ShuttleGamepadController(game::Shuttle* shuttle, mono::EventHandler& event_handler)
    : m_shuttle(shuttle),
      m_event_handler(event_handler)
{
    using namespace std::placeholders;
    using AddedFunc = std::function<bool (const event::ControllerAddedEvent&)>;
    using RemovedFunc = std::function<bool (const event::ControllerRemovedEvent&)>;

    const AddedFunc& add_func = std::bind(&ShuttleGamepadController::OnControllerAdded, this, _1);
    const RemovedFunc& remove_func = std::bind(&ShuttleGamepadController::OnControllerRemoved, this, _1);

    m_added_token = m_event_handler.AddListener(add_func);
    m_removed_token = m_event_handler.AddListener(remove_func);
}

ShuttleGamepadController::~ShuttleGamepadController()
{
    m_event_handler.RemoveListener(m_added_token);
    m_event_handler.RemoveListener(m_removed_token);
}

bool ShuttleGamepadController::OnControllerAdded(const event::ControllerAddedEvent& event)
{
    m_id = event.id;
    return false;
}

bool ShuttleGamepadController::OnControllerRemoved(const event::ControllerRemovedEvent& event)
{
    m_id = -1;
    return false;
}

void ShuttleGamepadController::Update(unsigned int delta)
{
    if(m_id == -1)
        return;

    const System::ControllerState& state = System::GetController(m_id);

    if(state.a)
        m_shuttle->Fire();
    else
        m_shuttle->StopFire();
        
    const math::Vector force(state.left_x, state.left_y);
    m_shuttle->ApplyImpulse(force * 10);

    std::printf("x: %f, y: %f\n", force.x, force.y);

    //m_shuttle->SelectWeapon(game::WeaponType::STANDARD);
    //m_event_handler.DispatchEvent(event::TimeScaleEvent(0.5f));
}
