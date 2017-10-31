
#include "InvaderController.h"
#include "EntityProperties.h"
#include "AIKnowledge.h"

#include "EventHandler/EventHandler.h"
#include "Rendering/Color.h"
#include "Rendering/Sprite/ISprite.h"

#include "Paths/PathFactory.h"
#include "Paths/IPath.h"

#include "Navigation/NavMesh.h"

using namespace game;

InvaderController::InvaderController(mono::EventHandler& event_handler)
{
    (void)event_handler;

    using namespace std::placeholders;

    const std::unordered_map<InvaderStates, InvaderStateMachine::State>& state_table = {
        { InvaderStates::IDLE,      { std::bind(&InvaderController::ToIdle,     this), std::bind(&InvaderController::Idle, this, _1) } },
        { InvaderStates::TRACKING,  { std::bind(&InvaderController::ToTracking, this), std::bind(&InvaderController::Tracking, this, _1) } }
    };

    m_states.SetStateTable(state_table);
}

void InvaderController::Initialize(Enemy* enemy)
{
    m_enemy = enemy;
    m_enemy->SetProperty(EntityProperties::DAMAGABLE);
    m_states.TransitionTo(InvaderStates::IDLE);
}

void InvaderController::doUpdate(unsigned int delta)
{
    m_states.UpdateState(delta);
}

void InvaderController::ToIdle()
{
    constexpr mono::Color::RGBA color(0.2, 0.2, 0.2);
    m_enemy->m_sprite->SetShade(color);

    m_idle_timer = 0;
}

void InvaderController::ToTracking()
{
    if(!game::player_one.is_active)
    {
        m_states.TransitionTo(InvaderStates::IDLE);
        return;
    }

    const int start = game::FindClosestIndex(*g_navmesh, m_enemy->Position());
    const int end = game::FindClosestIndex(*g_navmesh, player_one.position);

    const std::vector<int>& nav_path = game::AStar(*g_navmesh, start, end);
    const std::vector<math::Vector>& points = PathToPoints(*g_navmesh, nav_path);

    m_path = mono::CreatePath(math::zeroVec, points);
    m_current_position = 0.0f;

    constexpr mono::Color::RGBA color(1.0f, 0.0f, 0.0f);
    m_enemy->m_sprite->SetShade(color);    
}

void InvaderController::Idle(unsigned int delta)
{
    m_idle_timer += delta;
    if(m_idle_timer > 2000)
        m_states.TransitionTo(InvaderStates::TRACKING);
}

void InvaderController::Tracking(unsigned int delta)
{
    constexpr float speed_mps = 5.0f;
    m_current_position += speed_mps * float(delta) / 1000.0f;
    
    if(m_current_position > m_path->Length())
    {
        m_states.TransitionTo(InvaderStates::IDLE);
        return;
    }

    const math::Vector& global_position = m_path->GetGlobalPosition();
    const math::Vector& path_position = m_path->GetPositionByLength(m_current_position);

    m_enemy->SetPosition(global_position + path_position);
}
