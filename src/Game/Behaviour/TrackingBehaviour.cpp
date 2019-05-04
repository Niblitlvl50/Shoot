
#include "TrackingBehaviour.h"

#include "AIKnowledge.h"
#include "Navigation/NavMesh.h"

#include "Paths/PathFactory.h"
#include "Paths/IPath.h"

#include "Physics/IBody.h"
#include "Physics/IConstraint.h"
#include "Physics/PhysicsSystem.h"

#include "Entity/EntityBase.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"
#include "Rendering/Sprite/ISprite.h"

#include <cassert>

namespace game
{
class AStarPathDrawer : public mono::EntityBase
{
public:

    AStarPathDrawer(const float& path_position)
        : m_path_position(path_position)
        , m_path(nullptr)
    { } 

    void Update(const mono::UpdateContext& update_context)
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

    void SetPath(const mono::IPath* new_path)
    {
        m_path = new_path;
    }

    const float& m_path_position;
    const mono::IPath* m_path;
};
}

using namespace game;

TrackingBehaviour::TrackingBehaviour(mono::IBody* body, mono::PhysicsSystem* physics_system)
    : m_entity_body(body)
    , m_physics_system(physics_system)
    , m_tracking_timer(100000)
    , m_current_position(0.0f)
    , m_meter_per_second(3.0f)
{
    assert(body->GetType() == mono::BodyType::DYNAMIC);

    m_control_body = m_physics_system->CreateKinematicBody();
    m_control_body->SetPosition(body->GetPosition());

    m_spring = m_physics_system->CreateSpring(m_control_body, body, 1.0f, 200.0f, 1.5f);
    m_astar_drawer = std::make_shared<AStarPathDrawer>(m_current_position);
    //m_event_handler.DispatchEvent(SpawnEntityEvent(m_astar_drawer, 4));
}

TrackingBehaviour::~TrackingBehaviour()
{
    m_physics_system->ReleaseKinematicBody(m_control_body);
    m_physics_system->ReleaseConstraint(m_spring);
}

void TrackingBehaviour::SetTrackingSpeed(float meter_per_second)
{
    m_meter_per_second = meter_per_second;
}

TrackingResult TrackingBehaviour::Run(uint32_t delta_ms)
{
    m_tracking_timer += delta_ms;

    if(m_tracking_timer > 2000)
    {
        const bool path_updated = UpdatePath();
        if(!path_updated)
            return TrackingResult::NO_PATH;

        m_tracking_timer = 0;
    }

    m_current_position += m_meter_per_second * float(delta_ms) / 1000.0f;
    
    if(m_current_position > m_path->Length())
        return TrackingResult::AT_TARGET;

    const math::Vector& global_position = m_path->GetGlobalPosition();
    const math::Vector& path_position = m_path->GetPositionByLength(m_current_position);
    m_control_body->SetPosition(global_position + path_position);

    return TrackingResult::TRACKING;
}

bool TrackingBehaviour::UpdatePath()
{
    if(!game::g_player_one.is_active)
        return false;

    const math::Vector position = m_entity_body->GetPosition();

    const int start = game::FindClosestIndex(*g_navmesh, position);
    const int end = game::FindClosestIndex(*g_navmesh, g_player_one.position);

    if(start == end || start == -1 || end == -1)
        return false;

    const std::vector<int>& nav_path = game::AStar(*g_navmesh, start, end);
    if(nav_path.empty())
        return false;

    const std::vector<math::Vector>& points = PathToPoints(*g_navmesh, nav_path);
    m_path = mono::CreatePath(math::ZeroVec, points);

    const float length = math::Length(position - m_control_body->GetPosition());
    m_current_position = length;

    m_astar_drawer->SetPath(m_path.get());

    return true;
}
