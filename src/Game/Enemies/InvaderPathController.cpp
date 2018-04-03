
#include "InvaderPathController.h"

#include "AIKnowledge.h"
#include "EntityProperties.h"
#include "Factories.h"
#include "RenderLayers.h"
#include "Weapons/IWeaponSystem.h"
#include "Weapons/IWeaponFactory.h"

#include "Events/SpawnConstraintEvent.h"
#include "Events/SpawnEntityEvent.h"

#include "Entity/EntityBase.h"
#include "EventHandler/EventHandler.h"

#include "Math/MathFunctions.h"
#include "Paths/IPath.h"

#include "Physics/CMFactory.h"
#include "Physics/IBody.h"
#include "Physics/IConstraint.h"

#include "Rendering/IRenderer.h"
#include "Rendering/Sprite/ISprite.h"

using namespace game;

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

InvaderPathController::InvaderPathController(const mono::IPathPtr& path, mono::EventHandler& event_handler)
    : m_path(path)
    , m_eventHandler(event_handler)
    , m_currentPosition(0.0f)
    , m_fireCount(0)
    , m_fireCooldown(0)
{ }

InvaderPathController::~InvaderPathController()
{
    m_eventHandler.DispatchEvent(DespawnConstraintEvent(m_spring));
}

void InvaderPathController::Initialize(Enemy* enemy)
{
    m_enemy = enemy;
    m_enemy->SetProperty(EntityProperties::DAMAGABLE);

    m_weapon = weapon_factory->CreateWeapon(WeaponType::GENERIC, WeaponFaction::ENEMY);

    m_controlBody = mono::PhysicsFactory::CreateKinematicBody();
    m_spring = mono::PhysicsFactory::CreateSpring(m_controlBody, m_enemy->GetPhysics().body, 1.0f, 20.0f, 0.5f);

    m_eventHandler.DispatchEvent(SpawnConstraintEvent(m_spring));
    //m_eventHandler.DispatchEvent(SpawnEntityEvent(std::make_shared<DotEntity>(m_point), LayerId::FOREGROUND, nullptr));
}

void InvaderPathController::doUpdate(unsigned int delta)
{
    constexpr float speed_mps = 2.0f;
    m_currentPosition += speed_mps * float(delta) / 1000.0f;

    const math::Vector& global_position = m_path->GetGlobalPosition();
    const math::Vector& path_position = m_path->GetPositionByLength(m_currentPosition);
    m_point = global_position + path_position;
    m_controlBody->SetPosition(m_point);

    if(m_currentPosition > m_path->Length())
        m_currentPosition = 0.0f;

    if(m_fireCooldown > 0)
    {
        m_fireCooldown -= delta;
        return;
    }

    if(!player_one.is_active)
        return;

    const math::Vector& enemy_position = m_enemy->Position();
    const bool is_visible = math::PointInsideQuad(enemy_position, camera_viewport);
    if(!is_visible)
        return;

    const float distance = math::Length(player_one.position - enemy_position);
    if(distance < 7.0f)
    {
        const float angle = math::AngleBetweenPoints(player_one.position, enemy_position) + math::PI_2();
        m_fireCount += (m_weapon->Fire(enemy_position, angle) == WeaponFireResult::FIRE);
    }

    if(m_fireCount == 5)
    {
        m_fireCooldown = 2000;
        m_fireCount = 0;
    }
}
