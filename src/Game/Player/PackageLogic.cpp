
#include "PackageLogic.h"
#include "Player/PlayerInfo.h"
#include "Events/PackageEvents.h"
#include "IDebugDrawer.h"
#include "CollisionConfiguration.h"
#include "DamageSystem.h"

#include "EventHandler/EventHandler.h"
#include "Physics/PhysicsSystem.h"
#include "Rendering/Color.h"
#include "SystemContext.h"
#include "TransformSystem/TransformSystem.h"

#include "System/System.h"

namespace tweak_values
{
    constexpr float shield_duration_s = 5.0f;
    constexpr float velocity_threshold = 2.0f;
}

using namespace game;

PackageLogic::PackageLogic(
    uint32_t entity_id, game::PackageInfo* package_info, mono::EventHandler* event_handler, mono::SystemContext* system_context)
    : m_entity_id(entity_id)
    , m_package_info(package_info)
    , m_event_handler(event_handler)
    , m_shield_timer_s(0.0f)
    , m_damage_on_impact(false)
{
    m_transform_system = system_context->GetSystem<mono::TransformSystem>();
    m_physics_system = system_context->GetSystem<mono::PhysicsSystem>();
    m_damage_system = system_context->GetSystem<game::DamageSystem>();

    mono::IBody* body = m_physics_system->GetBody(m_entity_id);
    body->AddCollisionHandler(this);

    const std::function<mono::EventResult (const PackagePickupEvent&)> pickup_func =
        std::bind(&PackageLogic::OnPackageEvent, this, std::placeholders::_1);
    m_pickup_event_token = m_event_handler->AddListener(pickup_func);
}

PackageLogic::~PackageLogic()
{
    m_event_handler->RemoveListener(m_pickup_event_token);
}

void PackageLogic::Update(const mono::UpdateContext& update_context)
{
    m_shield_timer_s = std::clamp(m_shield_timer_s - update_context.delta_s, 0.0f, 10.0f);
    m_package_info->shielded = (m_shield_timer_s > 0.0f);

    if(m_damage_on_impact)
    {
        const mono::IBody* body = m_physics_system->GetBody(m_entity_id);
        const float velocity = math::Length(body->GetVelocity());
        m_damage_on_impact = (velocity >= tweak_values::velocity_threshold);
    }
}

void PackageLogic::DrawDebugInfo(IDebugDrawer* debug_drawer) const
{
    const math::Vector position = m_transform_system->GetWorldPosition(m_entity_id);
    debug_drawer->DrawCircle(position, 1.0f, mono::Color::CYAN);
}

const char* PackageLogic::GetDebugCategory() const
{
    return "Package";
}

mono::CollisionResolve PackageLogic::OnCollideWith(
    mono::IBody* body, const math::Vector& collision_point, const math::Vector& collision_normal, uint32_t categories)
{
    const bool is_enemy = (categories & CollisionCategory::ENEMY);
    if(m_damage_on_impact && is_enemy)
    {
        const uint32_t body_id = mono::PhysicsSystem::GetIdFromBody(body);
        m_damage_system->ApplyDamage(body_id, 100, m_entity_id);
        m_damage_on_impact = false;
    }

    return mono::CollisionResolve::NORMAL;
}

void PackageLogic::OnSeparateFrom(mono::IBody* body)
{ }

mono::EventResult PackageLogic::OnPackageEvent(const PackagePickupEvent& event)
{
    if(event.package_id != m_entity_id)
        return mono::EventResult::PASS_ON;

    switch(event.action)
    {
    case PackageAction::PICKED_UP:
        System::Log("Picked Up!");
        break;
    case PackageAction::DROPPED:
        m_shield_timer_s = tweak_values::shield_duration_s;
        System::Log("Dropped!");
        break;
    case PackageAction::THROWN:
        m_damage_on_impact = true;
        System::Log("Thrown!");
        break;
    }

    return mono::EventResult::HANDLED;
}
