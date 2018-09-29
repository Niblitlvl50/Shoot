
#include "PathBehaviour.h"

#include "AIKnowledge.h"
#include "Enemies/Enemy.h"
#include "Events/SpawnConstraintEvent.h"
#include "Events/SpawnEntityEvent.h"
#include "RenderLayers.h"

#include "Entity/EntityBase.h"
#include "EventHandler/EventHandler.h"
#include "Paths/IPath.h"
#include "Physics/CMFactory.h"
#include "Physics/IBody.h"
#include "Physics/IConstraint.h"
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

PathBehaviour::PathBehaviour(Enemy* enemy, const mono::IPathPtr& path, mono::EventHandler& event_handler)
    : m_path(path)
    , m_event_handler(event_handler)
    , m_current_position(0.0f)
    , m_meter_per_second(2.0f)
{
    m_control_body = mono::PhysicsFactory::CreateKinematicBody();
    m_spring = mono::PhysicsFactory::CreateSpring(m_control_body, enemy->GetPhysics().body, 1.0f, 20.0f, 0.5f);

    m_event_handler.DispatchEvent(SpawnConstraintEvent(m_spring));
    //m_event_handler.DispatchEvent(SpawnEntityEvent(std::make_shared<DotEntity>(m_point), LayerId::FOREGROUND, nullptr));
}

PathBehaviour::~PathBehaviour()
{
    m_event_handler.DispatchEvent(DespawnConstraintEvent(m_spring));
    //m_event_handler.DispatchEvent(DeSpawnEntityEvent());
}

void PathBehaviour::SetTrackingSpeed(float meter_per_second)
{
    m_meter_per_second = meter_per_second;
}

void PathBehaviour::Run(unsigned int delta)
{
    m_current_position += m_meter_per_second * float(delta) / 1000.0f;

    const math::Vector& global_position = m_path->GetGlobalPosition();
    const math::Vector& path_position = m_path->GetPositionByLength(m_current_position);
    m_point = global_position + path_position;
    m_control_body->SetPosition(m_point);

    if(m_current_position > m_path->Length())
        m_current_position = 0.0f;
}
