
#include "PackageLogic.h"
#include "Player/PlayerInfo.h"
#include "Events/PackageEvents.h"
#include "IDebugDrawer.h"
#include "CollisionConfiguration.h"
#include "DamageSystem.h"

#include "EntitySystem/IEntityManager.h"
#include "EventHandler/EventHandler.h"
#include "Physics/PhysicsSystem.h"
#include "Rendering/Color.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Sprite/Sprite.h"
#include "SystemContext.h"
#include "TransformSystem/TransformSystem.h"

#include "System/System.h"

namespace tweak_values
{
    constexpr float shield_duration_s = 5.0f;
    constexpr float shield_cooldown_duration_s = 5.0f;
    constexpr float velocity_threshold = 2.0f;
}

namespace
{
    const char* StatesToString(game::PackageLogic::States state)
    {
        switch(state)
        {
        case game::PackageLogic::States::IDLE:
            return "Idle";
        case game::PackageLogic::States::SHIELDED:
            return "Shielded";
        case game::PackageLogic::States::THROWN:
            return "Thrown";
        }

        return "Unknown";
    }
}

using namespace game;

PackageLogic::PackageLogic(
    uint32_t entity_id, game::PackageInfo* package_info, mono::EventHandler* event_handler, mono::SystemContext* system_context)
    : m_entity_id(entity_id)
    , m_package_info(package_info)
    , m_event_handler(event_handler)
    , m_shield_timer_s(0.0f)
    , m_shield_cooldown_s(0.0f)
{
    m_transform_system = system_context->GetSystem<mono::TransformSystem>();
    m_sprite_system = system_context->GetSystem<mono::SpriteSystem>();
    m_physics_system = system_context->GetSystem<mono::PhysicsSystem>();
    m_entity_manager = system_context->GetSystem<mono::IEntityManager>();
    m_damage_system = system_context->GetSystem<game::DamageSystem>();

    mono::IBody* body = m_physics_system->GetBody(m_entity_id);
    body->AddCollisionHandler(this);

    mono::Entity spawned_entity = m_entity_manager->CreateEntity("res/entities/package_shield.entity");
    m_spawned_shield_id = spawned_entity.id;

    m_transform_system->ChildTransform(m_spawned_shield_id, m_entity_id);
    m_sprite_system->SetSpriteEnabled(m_spawned_shield_id, false);

    const std::function<mono::EventResult (const PackagePickupEvent&)> pickup_func =
        std::bind(&PackageLogic::OnPackageEvent, this, std::placeholders::_1);
    m_pickup_event_token = m_event_handler->AddListener(pickup_func);

    const PackageStateMachine::StateTable state_table = {
        PackageStateMachine::MakeState(States::IDLE, &PackageLogic::ToIdle, &PackageLogic::Idle, this),
        PackageStateMachine::MakeState(
            States::SHIELDED, &PackageLogic::ToShielded, &PackageLogic::Shielded, &PackageLogic::ExitShielded, this),
        PackageStateMachine::MakeState(States::THROWN, &PackageLogic::ToThrown, &PackageLogic::Thrown, this),
    };
    m_states.SetStateTableAndState(state_table, States::IDLE);
}

PackageLogic::~PackageLogic()
{
    m_entity_manager->ReleaseEntity(m_spawned_shield_id);
    m_event_handler->RemoveListener(m_pickup_event_token);
}

void PackageLogic::Update(const mono::UpdateContext& update_context)
{
    m_states.UpdateState(update_context);

    m_shield_timer_s = std::clamp(m_shield_timer_s - update_context.delta_s, 0.0f, 10.0f);
    m_shield_cooldown_s = std::clamp(m_shield_cooldown_s - update_context.delta_s, 0.0f, 10.0f);

    m_package_info->cooldown_fraction = (m_shield_cooldown_s / tweak_values::shield_cooldown_duration_s);
}

void PackageLogic::DrawDebugInfo(IDebugDrawer* debug_drawer) const
{
    const char* shielded_string = StatesToString(m_states.ActiveState());
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
    const bool in_throwing_state = (m_states.ActiveState() == States::THROWN);
    const bool is_enemy = (categories & CollisionCategory::ENEMY);
    if(in_throwing_state && is_enemy)
    {
        const uint32_t body_id = mono::PhysicsSystem::GetIdFromBody(body);
        m_damage_system->ApplyDamage(body_id, 100, m_entity_id);
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
        if(!IsShieldOnCooldown())
            m_states.TransitionTo(States::SHIELDED);
        break;
    case PackageAction::THROWN:
        m_states.TransitionTo(States::THROWN);
        break;
    }

    return mono::EventResult::HANDLED;
}

bool PackageLogic::IsShieldOnCooldown() const
{
    return m_shield_cooldown_s > 0.0f;
}

void PackageLogic::ToIdle()
{}

void PackageLogic::Idle(const mono::UpdateContext& update_context)
{}

void PackageLogic::ToShielded()
{
    m_sprite_system->SetSpriteEnabled(m_spawned_shield_id, true);
    m_shield_timer_s = tweak_values::shield_duration_s;
}

void PackageLogic::Shielded(const mono::UpdateContext& update_context)
{
    if(m_shield_timer_s <= 0.0f)
        m_states.TransitionTo(States::IDLE);
}

void PackageLogic::ExitShielded()
{
    m_sprite_system->SetSpriteEnabled(m_spawned_shield_id, false);
    m_shield_cooldown_s = tweak_values::shield_cooldown_duration_s;
}

void PackageLogic::ToThrown()
{}

void PackageLogic::Thrown(const mono::UpdateContext& update_context)
{
    const mono::IBody* body = m_physics_system->GetBody(m_entity_id);
    const float velocity = math::Length(body->GetVelocity());

    if(velocity < tweak_values::velocity_threshold)
    {
        const States new_state = IsShieldOnCooldown() ? States::IDLE : States::SHIELDED;
        m_states.TransitionTo(new_state);
    }
}
