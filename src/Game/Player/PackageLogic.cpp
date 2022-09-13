
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
    constexpr float shield_cooldown_duration_s = 5.0f;
    constexpr float velocity_threshold = 2.0f;
}

using namespace game;

PackageLogic::PackageLogic(
    uint32_t entity_id, game::PackageInfo* package_info, mono::EventHandler* event_handler, mono::SystemContext* system_context)
    : m_entity_id(entity_id)
    , m_package_info(package_info)
    , m_event_handler(event_handler)
    , m_shield_timer_s(0.0f)
    , m_shield_cooldown_s(0.0f)
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
    m_shield_cooldown_s = std::clamp(m_shield_cooldown_s - update_context.delta_s, 0.0f, 10.0f);

    const bool was_shielded = m_package_info->shielded;

    const bool enable_shield = (m_shield_timer_s > 0.0f && m_shield_cooldown_s == 0.0f);
    m_package_info->shielded = enable_shield;

    if(was_shielded && !enable_shield)
        m_shield_cooldown_s = tweak_values::shield_cooldown_duration_s;

    m_package_info->cooldown_fraction = (m_shield_cooldown_s / tweak_values::shield_cooldown_duration_s);

    if(m_damage_on_impact)
    {
        const mono::IBody* body = m_physics_system->GetBody(m_entity_id);
        const float velocity = math::Length(body->GetVelocity());
        m_damage_on_impact = (velocity >= tweak_values::velocity_threshold);

        if(m_shield_cooldown_s == 0.0f)
            m_shield_timer_s = tweak_values::shield_duration_s;
    }
}

void PackageLogic::DrawDebugInfo(IDebugDrawer* debug_drawer) const
{
    const char* shielded_string = m_package_info->shielded ? "true" : "false";
    const std::string text = shielded_string + std::string(" ") + std::to_string(m_shield_timer_s) + "/" + std::to_string(m_shield_cooldown_s);
    debug_drawer->DrawScreenText(text.c_str(), math::Vector(1.0f, 3.0f), mono::Color::BLACK);
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
        break;
    case PackageAction::DROPPED:
    {
        if(m_shield_cooldown_s == 0.0f)
            m_shield_timer_s = tweak_values::shield_duration_s;
        break;
    }
    case PackageAction::THROWN:
        m_damage_on_impact = true;
        break;
    }

    return mono::EventResult::HANDLED;
}
