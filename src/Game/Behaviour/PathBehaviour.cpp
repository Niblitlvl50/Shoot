
#include "PathBehaviour.h"

#include "AIKnowledge.h"

#include "Paths/IPath.h"

#include "Physics/IBody.h"
#include "Physics/IConstraint.h"
#include "Physics/PhysicsSystem.h"

#include "Entity/EntityBase.h"
#include "Rendering/Color.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Sprite/ISprite.h"


namespace
{
    class DotEntity : public mono::EntityBase
    {
    public:

        DotEntity(const math::Vector& point)
            : m_point(point)
        {
            m_scale = math::Vector(10, 10);
        }

        virtual void Draw(mono::IRenderer& renderer) const
        {
            constexpr mono::Color::RGBA color(1.0f, 0.5f, 1.0f, 1.0f);
            renderer.DrawPoints({ math::ZeroVec }, color, 10.0f);
        }

        virtual void Update(unsigned int delta)
        {
            m_position = m_point;
        }
        
        const math::Vector& m_point;
    };
}

using namespace game;

PathBehaviour::PathBehaviour(
    mono::IBody* entity_body, const mono::IPath* path, mono::PhysicsSystem* physics_system, mono::EventHandler& event_handler)
    : m_path(path)
    , m_physics_system(physics_system)
    , m_current_position(0.0f)
    , m_meter_per_second(2.0f)
{
    //assert(body->GetType() == mono::BodyType::DYNAMIC);

    m_control_body = m_physics_system->CreateKinematicBody();
    m_spring = m_physics_system->CreateSpring(m_control_body, entity_body, 1.0f, 20.0f, 0.5f);

    //m_event_handler.DispatchEvent(SpawnEntityEvent(std::make_shared<DotEntity>(m_point), LayerId::FOREGROUND, nullptr));
}

PathBehaviour::~PathBehaviour()
{
    m_physics_system->ReleaseConstraint(m_spring);
    m_physics_system->ReleaseKinematicBody(m_control_body);
}

void PathBehaviour::SetTrackingSpeed(float meter_per_second)
{
    m_meter_per_second = meter_per_second;
}

void PathBehaviour::Run(uint32_t delta_ms)
{
    m_current_position += m_meter_per_second * float(delta_ms) / 1000.0f;

    const math::Vector& global_position = m_path->GetGlobalPosition();
    const math::Vector& path_position = m_path->GetPositionByLength(m_current_position);
    m_point = global_position + path_position;
    m_control_body->SetPosition(m_point);

    if(m_current_position > m_path->Length())
        m_current_position = 0.0f;
}
