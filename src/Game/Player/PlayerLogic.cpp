
#include "PlayerLogic.h"
#include "Player/PlayerInfo.h"

#include "SystemContext.h"
#include "TransformSystem/TransformSystem.h"
#include "Physics/PhysicsSystem.h"
#include "Rendering/Sprite/Sprite.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Sprite/SpriteProperties.h"
#include "Particle/ParticleSystem.h"

#include "DamageSystem.h"
#include "InteractionSystem/InteractionSystem.h"
#include "Factories.h"
#include "Weapons/IWeapon.h"
#include "Weapons/IWeaponFactory.h"

#include "EntitySystem/IEntityManager.h"
#include "Math/MathFunctions.h"

#include "Effects/TrailEffect.h"
#include "Effects/BlinkEffect.h"
#include "Pickups/PickupSystem.h"

#include <cmath>

namespace tweak_values
{
    constexpr float force_multiplier = 250.0f;
    constexpr uint32_t blink_duration_ms = 200;
    constexpr float blink_distance = 2.0f;
}

using namespace game;

PlayerLogic::PlayerLogic(
    uint32_t entity_id,
    PlayerInfo* player_info,
    mono::EventHandler* event_handler,
    const System::ControllerState& controller,
    mono::SystemContext* system_context)
    : m_entity_id(entity_id)
    , m_controller_id(controller.id)
    , m_player_info(player_info)
    , m_gamepad_controller(this, event_handler, controller)
    , m_fire(false)
    , m_total_ammo_left(500)
    , m_aim_direction(0.0f)
    , m_picked_up_id(mono::INVALID_ID)
    , m_pickup_constraint(nullptr)
{
    m_transform_system = system_context->GetSystem<mono::TransformSystem>();
    m_physics_system = system_context->GetSystem<mono::PhysicsSystem>();
    m_sprite_system = system_context->GetSystem<mono::SpriteSystem>();
    m_entity_system = system_context->GetSystem<mono::IEntityManager>();
    m_damage_system = system_context->GetSystem<DamageSystem>();
    m_pickup_system = system_context->GetSystem<PickupSystem>();
    m_interaction_system = system_context->GetSystem<InteractionSystem>();

    mono::ISprite* sprite = m_sprite_system->GetSprite(entity_id);
    m_idle_anim_id = sprite->GetAnimationIdFromName("idle");
    m_run_anim_id = sprite->GetAnimationIdFromName("run");

    using namespace std::placeholders;
    game::PickupCallback handle_pickups = std::bind(&PlayerLogic::HandlePickup, this, _1, _2);
    m_pickup_system->RegisterPickupTarget(m_entity_id, handle_pickups);

    mono::ParticleSystem* particle_system = system_context->GetSystem<mono::ParticleSystem>();
    m_blink_effect = std::make_unique<BlinkEffect>(particle_system, m_entity_system);
    m_blink_sound = audio::CreateSound("res/sound/punch.wav", audio::SoundPlayback::ONCE);

    const mono::Entity spawned_weapon = m_entity_system->CreateEntity("res/entities/player_weapon.entity");
    m_weapon_entity = spawned_weapon.id;

    m_transform_system->ChildTransform(m_weapon_entity, m_entity_id);

    // Make sure we have a weapon
    SelectWeapon(game::PLASMA_GUN);
    SetAimDirection(math::PI_2());

    const PlayerStateMachine::StateTable state_table = {
        PlayerStateMachine::MakeState(PlayerStates::DEFAULT, &PlayerLogic::ToDefault, &PlayerLogic::DefaultState, this),
        PlayerStateMachine::MakeState(PlayerStates::DEAD, &PlayerLogic::ToDead, &PlayerLogic::DeadState, this),
        PlayerStateMachine::MakeState(PlayerStates::BLINK, &PlayerLogic::ToBlink, &PlayerLogic::BlinkState, this),
    };

    m_state.SetStateTableAndState(state_table, PlayerStates::DEFAULT);
}

PlayerLogic::~PlayerLogic()
{
    m_pickup_system->UnregisterPickupTarget(m_entity_id);
    m_entity_system->ReleaseEntity(m_weapon_entity);
}

void PlayerLogic::Update(const mono::UpdateContext& update_context)
{
    m_state.UpdateState(update_context);
}

void PlayerLogic::UpdateAnimation(float aim_direction, const math::Vector& player_velocity)
{
    float anim_speed = 1.0f;
    int anim_id = m_idle_anim_id;

    const bool facing_left = (aim_direction < math::PI());
    const float velocity_magnitude = math::Length(player_velocity);

    if(velocity_magnitude > 0.2f)
    {
        anim_id = m_run_anim_id;
        anim_speed = std::clamp(math::Scale01(velocity_magnitude, 0.0f, 5.0f), 0.5f, 10.0f);
    }

    mono::Sprite* sprite = m_sprite_system->GetSprite(m_entity_id);
    if(facing_left)
        sprite->SetProperty(mono::SpriteProperty::FLIP_HORIZONTAL);
    else
        sprite->ClearProperty(mono::SpriteProperty::FLIP_HORIZONTAL);

    if(anim_id != sprite->GetActiveAnimation())
        sprite->SetAnimation(anim_id);
    sprite->SetAnimationPlaybackSpeed(anim_speed);
}

void PlayerLogic::ToDefault()
{
    m_sprite_system->SetSpriteEnabled(m_entity_id, true);
    m_sprite_system->SetSpriteEnabled(m_weapon_entity, true);
}

void PlayerLogic::DefaultState(const mono::UpdateContext& update_context)
{
    m_gamepad_controller.Update(update_context);

    const math::Matrix& transform = m_transform_system->GetWorld(m_entity_id);
    const math::Vector& position = math::GetPosition(transform);
    const float direction = math::GetZRotation(transform);

    if(m_fire)
    {
        const math::Vector offset = math::Normalized(math::VectorFromAngle(m_aim_direction)) * 0.5f;
        m_player_info->weapon_state = m_weapon->Fire(position + offset, m_aim_direction, update_context.timestamp);
    }

    mono::IBody* body = m_physics_system->GetBody(m_entity_id);

    m_player_info->position = position;
    m_player_info->velocity = body->GetVelocity();
    m_player_info->direction = direction;

    m_player_info->weapon_type = m_weapon_type;
    m_player_info->weapon_state = m_weapon->UpdateWeaponState(update_context.timestamp);
    m_player_info->weapon_reload_percentage = m_weapon->ReloadPercentage();
    m_player_info->magazine_capacity = m_weapon->MagazineSize();
    m_player_info->magazine_left = m_weapon->AmmunitionLeft();
    m_player_info->ammunition_left = m_total_ammo_left;

    UpdateAnimation(m_aim_direction, m_player_info->velocity);

    if(m_player_info->player_state == PlayerState::DEAD)
        m_state.TransitionTo(PlayerStates::DEAD);
}

void PlayerLogic::ToDead()
{
    m_sprite_system->SetSpriteEnabled(m_entity_id, false);
    m_sprite_system->SetSpriteEnabled(m_weapon_entity, false);
}

void PlayerLogic::DeadState(const mono::UpdateContext& update_context)
{
    if(m_player_info->player_state == PlayerState::ALIVE)
        m_state.TransitionTo(PlayerStates::DEFAULT);
}

void PlayerLogic::ToBlink()
{
    m_sprite_system->SetSpriteEnabled(m_entity_id, false);
    m_sprite_system->SetSpriteEnabled(m_weapon_entity, false);

    const math::Matrix& transform = m_transform_system->GetWorld(m_entity_id);
    const math::Vector& position = math::GetPosition(transform);

    m_blink_effect->EmitBlinkAwayAt(position);
    m_blink_sound->Play();

    m_blink_counter = 0;
}

void PlayerLogic::BlinkState(const mono::UpdateContext& update_context)
{
    m_blink_counter += update_context.delta_ms;

    if(m_blink_counter >= tweak_values::blink_duration_ms)
    {
        mono::IBody* body = m_physics_system->GetBody(m_entity_id);

        const math::Vector new_position = body->GetPosition() + (m_blink_direction * tweak_values::blink_distance);
        body->SetPosition(new_position);

        m_blink_effect->EmitBlinkBackAt(new_position);
        m_state.TransitionTo(PlayerStates::DEFAULT);
    }
}

void PlayerLogic::Fire()
{
    m_fire = true;
}

void PlayerLogic::StopFire()
{
    m_fire = false;
}

void PlayerLogic::Reload(uint32_t timestamp)
{
    /*
    m_total_ammo_left -= m_weapon->MagazineSize() + m_weapon->AmmunitionLeft();
    m_total_ammo_left = std::max(0, m_total_ammo_left);

    if(m_total_ammo_left != 0)
        m_weapon->Reload(timestamp);
    */

    m_weapon->Reload(timestamp);
}

void PlayerLogic::UseItemSlot(ItemSlotIndex slot_index)
{
    const bool can_trigger_interaction = m_interaction_system->CanPlayerTriggerInteraction(m_entity_id);
    if(can_trigger_interaction)
    {

    }

    ItemSlot& item_slot = m_item_slots[slot_index];
}

void PlayerLogic::HandlePickup(shared::PickupType type, int amount)
{
    switch(type)
    {
    case shared::PickupType::AMMO:
        m_total_ammo_left += amount;
        break;
    case shared::PickupType::HEALTH:
    {
        DamageRecord* record = m_damage_system->GetDamageRecord(m_entity_id);
        record->health += amount;
        break;
    }
    case shared::PickupType::SCORE:
        m_player_info->score += amount;
        break;

    case shared::PickupType::WEAPON_PISTOL:
    case shared::PickupType::WEAPON_PLASMA:
    case shared::PickupType::WEAPON_SHOTGUN:
        HandleWeaponPickup(type);
        break;
    };
}

void PlayerLogic::Throw()
{
    if(m_picked_up_id != mono::INVALID_ID)
    {
        mono::IBody* body = m_physics_system->GetBody(m_picked_up_id);
        if(body)
        {
            m_physics_system->ReleaseConstraint(m_pickup_constraint);
            m_pickup_constraint = nullptr;

            const math::Vector throw_direction = math::Normalized(math::VectorFromAngle(m_aim_direction));
            body->ApplyLocalImpulse(throw_direction * 100.0f, math::ZeroVec);
        }

        m_interaction_system->SetInteractionEnabled(m_picked_up_id, true);
        m_picked_up_id = mono::INVALID_ID;
    }
}

void PlayerLogic::PickupDrop()
{
    if(m_picked_up_id == mono::INVALID_ID)
    {
        const InteractionCallback interaction_callback = [this](uint32_t interaction_id, shared::InteractionType interaction_type) {
            if(interaction_type == shared::InteractionType::PICKUP)
            {
                m_interaction_system->SetInteractionEnabled(interaction_id, false);
                m_picked_up_id = interaction_id;

                mono::IBody* player_body = m_physics_system->GetBody(m_entity_id);
                mono::IBody* pickup_body = m_physics_system->GetBody(m_picked_up_id);

                m_pickup_constraint = m_physics_system->CreateSpring(player_body, pickup_body, 0.25f, 100.0f, 0.5f);
            }
        };
        m_interaction_system->TryTriggerInteraction(m_entity_id, interaction_callback);
    }
    else
    {
        m_interaction_system->SetInteractionEnabled(m_picked_up_id, true);
        m_picked_up_id = mono::INVALID_ID;

        m_physics_system->ReleaseConstraint(m_pickup_constraint);
        m_pickup_constraint = nullptr;
    }
}

void PlayerLogic::TriggerInteraction()
{
}

void PlayerLogic::SelectWeapon(WeaponSetup weapon)
{
    m_weapon = g_weapon_factory->CreateWeapon(weapon, WeaponFaction::PLAYER, m_entity_id);
    m_weapon_type = weapon;
}

void PlayerLogic::HandleWeaponPickup(shared::PickupType type)
{
    static const std::unordered_map<shared::PickupType, game::WeaponSetup> g_pickup_to_weapon = {
        { shared::PickupType::WEAPON_PISTOL,    game::GENERIC },
        { shared::PickupType::WEAPON_PLASMA,    game::PLASMA_GUN },
        { shared::PickupType::WEAPON_SHOTGUN,   game::FLAK_CANON },
    };

    static const std::unordered_map<uint32_t, const char*> g_weapon_to_entity = {
        { game::PLASMA_GUN.weapon_hash, "res/entities/plasma_gun_pickup.entity" },
        { game::GENERIC.weapon_hash,    "res/entities/rocket_launcher_pickup.entity" },
        { game::FLAK_CANON.weapon_hash, "res/entities/flak_cannon_pickup.entity" },
    };

    const auto it = g_pickup_to_weapon.find(type);
    if(it == g_pickup_to_weapon.end())
        return;

    const WeaponSetup weapon_type = it->second;

    m_weapon = g_weapon_factory->CreateWeapon(weapon_type, WeaponFaction::PLAYER, m_entity_id);

    const auto it_second = g_weapon_to_entity.find(m_weapon_type.weapon_hash);
    if(it_second != g_weapon_to_entity.end())
    {
        const math::Matrix& player_transform = m_transform_system->GetTransform(m_entity_id);

        mono::Entity spawned_entity = m_entity_system->CreateEntity(it_second->second);
        math::Matrix& transform = m_transform_system->GetTransform(spawned_entity.id);
        math::Position(transform, math::GetPosition(player_transform) + math::Vector(1.0f, 1.0f));

        m_transform_system->SetTransformState(spawned_entity.id, mono::TransformState::CLIENT);
    }

    m_weapon_type = weapon_type;
}

void PlayerLogic::MoveInDirection(const math::Vector& direction)
{
    const float length_squared = math::LengthSquared(direction);
    if(length_squared <= FLT_EPSILON)
        ResetForces();
    else
        ApplyForce(direction * tweak_values::force_multiplier);
}

void PlayerLogic::ApplyImpulse(const math::Vector& force)
{
    mono::IBody* body = m_physics_system->GetBody(m_entity_id);
    body->ApplyLocalImpulse(force, math::ZeroVec);
}

void PlayerLogic::ApplyForce(const math::Vector& force)
{
    const float multiplier = m_fire ? 0.5f : 1.0;
    mono::IBody* body = m_physics_system->GetBody(m_entity_id);
    body->ApplyLocalForce(force * multiplier, math::ZeroVec);
}

void PlayerLogic::SetVelocity(const math::Vector& velocity)
{
    mono::IBody* body = m_physics_system->GetBody(m_entity_id);
    body->SetVelocity(velocity);
}

void PlayerLogic::ResetForces()
{
    //mono::IBody* body = m_physics_system->GetBody(m_entity_id);
    //body->ResetForces();
    //body->SetVelocity(math::ZeroVec);

    //SetAnimation(PlayerAnimation::IDLE);
}

void PlayerLogic::SetAimDirection(float aim_direction)
{
    m_aim_direction = aim_direction;

    mono::Sprite* sprite = m_sprite_system->GetSprite(m_weapon_entity);
    if(aim_direction < math::PI())
        sprite->SetProperty(mono::SpriteProperty::FLIP_VERTICAL);
    else
        sprite->ClearProperty(mono::SpriteProperty::FLIP_VERTICAL);

    math::Matrix& weapon_transform = m_transform_system->GetTransform(m_weapon_entity);
    weapon_transform =
        math::CreateMatrixWithPosition(math::Vector(0.0f, -0.1f)) *
        math::CreateMatrixFromZRotation(aim_direction + math::PI_2()) *
        math::CreateMatrixWithPosition(math::Vector(0.2f, 0.0f));
}

void PlayerLogic::Blink(const math::Vector& direction)
{
    m_blink_direction = direction;
    m_state.TransitionTo(PlayerStates::BLINK);
}
