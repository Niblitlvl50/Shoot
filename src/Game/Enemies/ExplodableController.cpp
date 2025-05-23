
#include "ExplodableController.h"

#include "DamageSystem/DamageSystem.h"
#include "Effects/ExplosionEffect.h"
#include "DamageSystem/Shockwave.h"
#include "CollisionConfiguration.h"

#include "System/Audio.h"
#include "EntitySystem/IEntityManager.h"
#include "Particle/ParticleSystem.h"
#include "Physics/PhysicsSystem.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "SystemContext.h"
#include "TransformSystem/TransformSystem.h"

#include "Debug/IDebugDrawer.h"

namespace tweak_values
{
    constexpr float magnitude = 2.5f;
    constexpr float shockwave_radius = 1.5f;
    constexpr int damage = 25;
    constexpr float wait_duration = 0.5f;
}

using namespace game;

ExplodableController::ExplodableController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler* event_handler)
    : m_entity_id(entity_id)
    , m_draw_explosion_once(false)
{
    m_explosion_sound =
        audio::CreateSound("res/sound/explosion_metallic.wav", audio::SoundPlayback::ONCE);

    m_transform_system = system_context->GetSystem<mono::TransformSystem>();
    m_physics_system = system_context->GetSystem<mono::PhysicsSystem>();
    m_sprite_system = system_context->GetSystem<mono::SpriteSystem>();
    m_entity_system = system_context->GetSystem<mono::IEntityManager>();

    const DamageCallback destroyed_callback = [this](uint32_t damaged_entity_id, uint32_t who_did_damage, uint32_t weapon_identifier, int damage, DamageType type) {
        m_states.TransitionTo(States::DEAD);
    };

    m_damage_system = system_context->GetSystem<game::DamageSystem>();

    DamageRecord* damage_record = m_damage_system->GetDamageRecord(entity_id);
    damage_record->release_entity_on_death = false;

    m_damage_system->SetDamageCallback(entity_id, DamageType::DESTROYED, destroyed_callback);

    mono::ParticleSystem* particle_system = system_context->GetSystem<mono::ParticleSystem>();
    m_explosion_effect = std::make_unique<ExplosionEffect>(particle_system, m_entity_system);

    const ExplodableStateMachine::StateTable state_table = {
        ExplodableStateMachine::MakeState(States::IDLE, &ExplodableController::OnIdle, &ExplodableController::Idle, this),
        ExplodableStateMachine::MakeState(States::DEAD, &ExplodableController::OnDead, &ExplodableController::Dead, this),
    };

    m_states.SetStateTableAndState(state_table, States::IDLE);
}

void ExplodableController::DrawDebugInfo(IDebugDrawer* debug_drawer) const
{
    if(m_draw_explosion_once)
    {
        m_draw_explosion_once = false;

        const math::Vector world_position = m_transform_system->GetWorldPosition(m_entity_id);
        debug_drawer->DrawCircleFading(world_position, tweak_values::shockwave_radius, mono::Color::RED, 2.0f);
    }
}

const char* ExplodableController::GetDebugCategory() const
{
    return "Explodable Thing";
}

void ExplodableController::Update(const mono::UpdateContext& update_context)
{
    m_states.UpdateState(update_context);
}

void ExplodableController::OnIdle()
{ }

void ExplodableController::Idle(const mono::UpdateContext& update_context)
{ }

void ExplodableController::OnDead()
{
    m_explosion_sound->Play();
    m_sprite_system->SetSpriteEnabled(m_entity_id, false);

    const math::Vector world_position = m_transform_system->GetWorldPosition(m_entity_id);
    m_explosion_effect->ExplodeAt(world_position);

    game::ShockwaveAndDamageAt(
        m_physics_system,
        m_damage_system,
        world_position,
        tweak_values::magnitude,
        tweak_values::shockwave_radius,
        tweak_values::damage,
        m_entity_id,
        CollisionCategory::CC_ALL);

    m_wait_timer_s = 0.0f;

    m_draw_explosion_once = true;
}

void ExplodableController::Dead(const mono::UpdateContext& update_context)
{
    m_wait_timer_s += update_context.delta_s;
    if(m_wait_timer_s > tweak_values::wait_duration)
        m_entity_system->ReleaseEntity(m_entity_id);
}
