
#include "ExplodableController.h"

#include "DamageSystem.h"
#include "Effects/ExplosionEffect.h"
#include "Shockwave.h"
#include "CollisionConfiguration.h"

#include "System/Audio.h"
#include "EntitySystem/IEntityManager.h"
#include "Particle/ParticleSystem.h"
#include "Physics/PhysicsSystem.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "SystemContext.h"
#include "TransformSystem/TransformSystem.h"

using namespace game;

ExplodableController::ExplodableController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler& event_handler)
    : m_entity_id(entity_id)
{
    m_explosion_sound =
        audio::CreateSound("res/sound/explosion_metallic.wav", audio::SoundPlayback::ONCE);

    m_transform_system = system_context->GetSystem<mono::TransformSystem>();
    m_physics_system = system_context->GetSystem<mono::PhysicsSystem>();
    m_sprite_system = system_context->GetSystem<mono::SpriteSystem>();
    m_entity_system = system_context->GetSystem<mono::IEntityManager>();

    const DamageCallback destroyed_callback = [this](uint32_t id, int damage, uint32_t who_did_damage, DamageType type) {
        m_states.TransitionTo(States::DEAD);
    };

    m_damage_system = system_context->GetSystem<game::DamageSystem>();
    m_damage_system->PreventReleaseOnDeath(entity_id, true);
    uint32_t callback_id = m_damage_system->SetDamageCallback(entity_id, DamageType::DESTROYED, destroyed_callback);

    mono::ParticleSystem* particle_system = system_context->GetSystem<mono::ParticleSystem>();
    m_explosion_effect = std::make_unique<ExplosionEffect>(particle_system, m_entity_system);

    const ExplodableStateMachine::StateTable state_table = {
        ExplodableStateMachine::MakeState(States::IDLE, &ExplodableController::OnIdle, &ExplodableController::Idle, this),
        ExplodableStateMachine::MakeState(States::DEAD, &ExplodableController::OnDead, &ExplodableController::Dead, this),
    };

    m_states.SetStateTableAndState(state_table, States::IDLE);
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

    const math::Vector world_position = math::GetPosition(m_transform_system->GetWorld(m_entity_id));
    m_explosion_effect->ExplodeAt(world_position);

    game::ShockwaveAndDamageAt(
        m_physics_system, m_damage_system, world_position, 15.0f, 10, m_entity_id, CollisionCategory::CC_ALL);

    m_wait_timer = 0;
}

void ExplodableController::Dead(const mono::UpdateContext& update_context)
{
    m_wait_timer += update_context.delta_ms;
    if(m_wait_timer > 500)
        m_entity_system->ReleaseEntity(m_entity_id);
}
