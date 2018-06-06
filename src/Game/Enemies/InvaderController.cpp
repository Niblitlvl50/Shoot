
#include "InvaderController.h"
#include "EntityProperties.h"
#include "AIKnowledge.h"

#include "Factories.h"
#include "Weapons/IWeaponSystem.h"
#include "Weapons/IWeaponFactory.h"

#include "EventHandler/EventHandler.h"
#include "Rendering/Color.h"
#include "Rendering/Sprite/ISprite.h"
#include "Rendering/IRenderer.h"

#include "Paths/PathFactory.h"
#include "Paths/IPath.h"

#include "Physics/CMFactory.h"
#include "Physics/IBody.h"
#include "Physics/IConstraint.h"

#include "Entity/EntityBase.h"
#include "Math/MathFunctions.h"

#include "Events/SpawnConstraintEvent.h"
#include "Events/SpawnEntityEvent.h"
#include "Events/RemoveEntityEvent.h"
#include "Navigation/NavMesh.h"

namespace game
{
class AStarPathDrawer : public mono::EntityBase
{
public:

    AStarPathDrawer(const float& path_position)
        : m_path_position(path_position)
    { } 

    void Update(unsigned int delta)
    { }

    void Draw(mono::IRenderer& renderer) const
    {
        if(!m_path)
            return;
    
        constexpr mono::Color::RGBA path_color(1.0f, 0.0f, 0.0f);
        renderer.DrawPolyline(m_path->GetPathPoints(), path_color, 1.0f);

        constexpr mono::Color::RGBA point_color(0.0f, 1.0f, 1.0f);
        renderer.DrawPoints({m_path->GetPositionByLength(m_path_position)}, point_color, 4.0f);
    }

    math::Quad BoundingBox() const
    {
        return math::InfQuad;
    }

    void SetPath(const mono::IPathPtr& new_path)
    {
        m_path = new_path;
    }

    mono::IPathPtr m_path;
    const float& m_path_position;
};
}


using namespace game;

InvaderController::InvaderController(mono::EventHandler& event_handler)
    : m_event_handler(event_handler)
{
    using namespace std::placeholders;

    const std::unordered_map<InvaderStates, InvaderStateMachine::State>& state_table = {
        { InvaderStates::IDLE,      { std::bind(&InvaderController::ToIdle,      this), std::bind(&InvaderController::Idle,      this, _1) } },
        { InvaderStates::TRACKING,  { std::bind(&InvaderController::ToTracking,  this), std::bind(&InvaderController::Tracking,  this, _1) } },
        { InvaderStates::ATTACKING, { std::bind(&InvaderController::ToAttacking, this), std::bind(&InvaderController::Attacking, this, _1) } }
    };

    m_states.SetStateTable(state_table);
}

InvaderController::~InvaderController()
{
    m_event_handler.DispatchEvent(DespawnConstraintEvent(m_spring));    
    m_event_handler.DispatchEvent(RemoveEntityEvent(m_astar_drawer->Id()));
}

void InvaderController::Initialize(Enemy* enemy)
{
    m_enemy = enemy;
    m_enemy->SetProperty(EntityProperties::DAMAGABLE);

    m_weapon = weapon_factory->CreateWeapon(WeaponType::GENERIC, WeaponFaction::ENEMY);

    m_control_body = mono::PhysicsFactory::CreateKinematicBody();
    m_control_body->SetPosition(m_enemy->Position());

    m_spring = mono::PhysicsFactory::CreateSpring(m_control_body, m_enemy->GetPhysics().body, 1.0f, 200.0f, 1.5f);
    m_event_handler.DispatchEvent(SpawnConstraintEvent(m_spring));

    m_astar_drawer = std::make_shared<AStarPathDrawer>(m_current_position);
    //m_event_handler.DispatchEvent(SpawnEntityEvent(m_astar_drawer, 4, nullptr));

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
    const bool path_updated = UpdatePath();
    if(!path_updated)
    {
        m_states.TransitionTo(InvaderStates::IDLE);
        return;
    }

    constexpr mono::Color::RGBA color(1.0f, 0.0f, 0.0f);
    m_enemy->m_sprite->SetShade(color);
    m_tracking_timer = 0;
}

void InvaderController::ToAttacking()
{

}

void InvaderController::Idle(unsigned int delta)
{
    m_idle_timer += delta;
    if(m_idle_timer > 2000)
        m_states.TransitionTo(InvaderStates::TRACKING);
}

void InvaderController::Tracking(unsigned int delta)
{
    m_tracking_timer += delta;

    if(m_tracking_timer > 2000)
    {
        const bool path_updated = UpdatePath();
        if(!path_updated)
        {
            m_states.TransitionTo(InvaderStates::IDLE);
            return;
        }

        m_tracking_timer = 0;
    }

    constexpr float speed_mps = 3.0f;
    m_current_position += speed_mps * float(delta) / 1000.0f;
    
    if(m_current_position > m_path->Length())
    {
        m_states.TransitionTo(InvaderStates::IDLE);
        return;
    }

    const math::Vector& global_position = m_path->GetGlobalPosition();
    const math::Vector& path_position = m_path->GetPositionByLength(m_current_position);
    m_control_body->SetPosition(global_position + path_position);

    const float distance_to_player = math::Length(g_player_one.position - m_enemy->Position());
    if(distance_to_player < 5.0f)
        m_states.TransitionTo(InvaderStates::ATTACKING);
}

void InvaderController::Attacking(unsigned int delta)
{
    const float distance_to_player = math::Length(g_player_one.position - m_enemy->Position());
    if(distance_to_player > 10.0f)
    {
        m_states.TransitionTo(InvaderStates::TRACKING);
        return;
    }

    const math::Vector& enemy_position = m_enemy->Position();
    const float angle = math::AngleBetweenPoints(g_player_one.position, enemy_position) + math::PI_2();
    m_weapon->Fire(enemy_position, angle);
}

bool InvaderController::UpdatePath()
{
    if(!game::g_player_one.is_active)
        return false;

    const int start = game::FindClosestIndex(*g_navmesh, m_enemy->Position());
    const int end = game::FindClosestIndex(*g_navmesh, g_player_one.position);

    if(start == end)
        return false;

    const std::vector<int>& nav_path = game::AStar(*g_navmesh, start, end);
    const std::vector<math::Vector>& points = PathToPoints(*g_navmesh, nav_path);

    m_path = mono::CreatePath(math::ZeroVec, points);

    const float length = math::Length(m_enemy->Position() - m_control_body->GetPosition());
    m_current_position = length;

    m_astar_drawer->SetPath(m_path);

    return true;
}
