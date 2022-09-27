
#include "PlayerLogic.h"
#include "DecoyLogic.h"
#include "Player/PlayerInfo.h"

#include "Entity/EntityLogicSystem.h"
#include "Entity/Component.h"
#include "Events/PackageEvents.h"

#include "SystemContext.h"
#include "TransformSystem/TransformSystem.h"
#include "Physics/PhysicsSystem.h"
#include "Physics/IShape.h"
#include "Physics/PhysicsSpace.h"
#include "Rendering/Sprite/Sprite.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Sprite/SpriteProperties.h"
#include "Rendering/Lights/LightSystem.h"
#include "Particle/ParticleSystem.h"

#include "DamageSystem.h"
#include "InteractionSystem/InteractionSystem.h"
#include "Weapons/IWeapon.h"
#include "Weapons/WeaponSystem.h"

#include "EntitySystem/IEntityManager.h"
#include "EventHandler/EventHandler.h"
#include "Math/MathFunctions.h"
#include "Math/CriticalDampedSpring.h"
#include "Util/Random.h"

#include "Effects/SmokeEffect.h"
#include "Effects/ShockwaveEffect.h"
#include "Effects/FootStepsEffect.h"
#include "Pickups/PickupSystem.h"
#include "Shockwave.h"

#include <cmath>

namespace tweak_values
{
    constexpr float force_multiplier = 250.0f;
    constexpr float blink_duration_s = 0.2f;
    constexpr float blink_distance = 2.0f;
    constexpr float blink_cooldown_threshold_s = 2.0f;
    constexpr float shockwave_cooldown_s = 2.0f;
    constexpr float shield_cooldown_s = 2.0f;

    constexpr float footstep_length = 0.25f;
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
    , m_event_handler(event_handler)
    , m_fire(false)
    , m_stop_fire(false)
    , m_aim_direction(0.0f)
    , m_aim_target(0.0f)
    , m_aim_velocity(0.0f)
    , m_weapon_index(0)
    , m_accumulated_step_distance(0.0f)
    , m_blink_cooldown(tweak_values::blink_cooldown_threshold_s)
    , m_shockwave_cooldown(tweak_values::shockwave_cooldown_s)
    , m_shield_cooldown(tweak_values::shield_cooldown_s)
    , m_picked_up_id(mono::INVALID_ID)
    , m_pickup_constraint(nullptr)
{
    m_transform_system = system_context->GetSystem<mono::TransformSystem>();
    m_physics_system = system_context->GetSystem<mono::PhysicsSystem>();
    m_sprite_system = system_context->GetSystem<mono::SpriteSystem>();
    m_light_system = system_context->GetSystem<mono::LightSystem>();
    m_entity_system = system_context->GetSystem<mono::IEntityManager>();
    m_damage_system = system_context->GetSystem<DamageSystem>();
    m_pickup_system = system_context->GetSystem<PickupSystem>();
    m_interaction_system = system_context->GetSystem<InteractionSystem>();
    m_weapon_system = system_context->GetSystem<game::WeaponSystem>();
    m_logic_system = system_context->GetSystem<game::EntityLogicSystem>();

    mono::ISprite* sprite = m_sprite_system->GetSprite(entity_id);
    m_idle_anim_id = sprite->GetAnimationIdFromName("idle");
    m_run_anim_id = sprite->GetAnimationIdFromName("run");
    m_death_anim_id = sprite->GetAnimationIdFromName("death");

    using namespace std::placeholders;
    game::PickupCallback handle_pickups = std::bind(&PlayerLogic::HandlePickup, this, _1, _2);
    m_pickup_system->RegisterPickupTarget(m_entity_id, handle_pickups);

    mono::ParticleSystem* particle_system = system_context->GetSystem<mono::ParticleSystem>();
    m_blink_sound = audio::CreateSound("res/sound/punch.wav", audio::SoundPlayback::ONCE);
    m_running_sound = audio::CreateSound("res/sound/running-cartoon-footstep_1.wav", audio::SoundPlayback::ONCE);
    m_running_sound->SetVolume(0.75f);

    m_smoke_effect = std::make_unique<SmokeEffect>(particle_system, m_entity_system);
    m_shockwave_effect = std::make_unique<ShockwaveEffect>(m_transform_system, particle_system, m_entity_system);
    m_footsteps_effect = std::make_unique<FootStepsEffect>(particle_system, m_entity_system);

    const mono::Entity spawned_weapon = m_entity_system->CreateEntity("res/entities/player_weapon.entity");
    m_weapon_entity = spawned_weapon.id;
    m_transform_system->ChildTransform(m_weapon_entity, m_entity_id);

    // Make sure we have a weapon
    m_weapons[0] = m_weapon_system->CreatePrimaryWeapon(entity_id, WeaponFaction::PLAYER);
    m_weapons[1] = m_weapon_system->CreateSecondaryWeapon(entity_id, WeaponFaction::PLAYER);
    SelectWeapon(WeaponSelection::Previous);

    m_aim_target = m_aim_direction = -math::PI_2();

    const PlayerStateMachine::StateTable state_table = {
        PlayerStateMachine::MakeState(PlayerStates::DEFAULT, &PlayerLogic::ToDefault, &PlayerLogic::DefaultState, this),
        PlayerStateMachine::MakeState(PlayerStates::DEAD, &PlayerLogic::ToDead, &PlayerLogic::DeadState, &PlayerLogic::ExitDead, this),
        PlayerStateMachine::MakeState(PlayerStates::BLINK, &PlayerLogic::ToBlink, &PlayerLogic::BlinkState, &PlayerLogic::ExitBlink, this),
    };
    m_state.SetStateTableAndState(state_table, PlayerStates::DEFAULT);

    // Make sure the player info is updated when constructed.
    UpdatePlayerInfo(0);
}

PlayerLogic::~PlayerLogic()
{
    m_pickup_system->UnregisterPickupTarget(m_entity_id);
    m_entity_system->ReleaseEntity(m_weapon_entity);
}

void PlayerLogic::Update(const mono::UpdateContext& update_context)
{
    m_state.UpdateState(update_context);

    m_blink_cooldown += update_context.delta_s;
    m_shockwave_cooldown += update_context.delta_s;
    m_shield_cooldown += update_context.delta_s;

    const IWeaponPtr& active_weapon = m_weapons[m_weapon_index];

    m_active_cooldowns[PlayerAbility::WEAPON_RELOAD] = float(active_weapon->ReloadPercentage()) / 100.0f;
    m_active_cooldowns[PlayerAbility::BLINK] = m_blink_cooldown / tweak_values::blink_cooldown_threshold_s;
    m_active_cooldowns[PlayerAbility::SHOCKWAVE] = m_shockwave_cooldown / tweak_values::shockwave_cooldown_s;
    m_active_cooldowns[PlayerAbility::SHIELD] = m_shield_cooldown / tweak_values::shield_cooldown_s;
    m_active_cooldowns[PlayerAbility::WEAPON_AMMUNITION] =
        float(active_weapon->AmmunitionLeft()) / float(active_weapon->MagazineSize());

    UpdatePlayerInfo(update_context.timestamp);

    if(m_player_info->auto_aim)
    {
        mono::PhysicsSpace* space = m_physics_system->GetSpace();
        mono::QueryResult query_result = space->QueryNearest(m_player_info->position, 3.0f, CollisionCategory::ENEMY);
        if(query_result.body)
        {
            const math::Vector delta_to_target = (query_result.body->GetPosition() - m_player_info->position);
            const float aim_direction = math::AngleFromVector(delta_to_target);
            SetAimDirection(aim_direction);
        }
    }
}

void PlayerLogic::UpdatePlayerInfo(uint32_t timestamp)
{
    const math::Matrix& transform = m_transform_system->GetWorld(m_entity_id);
    const math::Vector last_position = m_player_info->position;
    const math::Vector current_position = math::GetPosition(transform);

    m_accumulated_step_distance += math::DistanceBetween(last_position, current_position);

    mono::IBody* body = m_physics_system->GetBody(m_entity_id);

    m_player_info->position = current_position;
    m_player_info->velocity = body->GetVelocity();
    m_player_info->direction = math::GetZRotation(transform);
    m_player_info->aim_direction = math::VectorFromAngle(m_aim_direction);

    IWeaponPtr& active_weapon = m_weapons[m_weapon_index];

    m_player_info->weapon_type = active_weapon->GetWeaponSetup();
    m_player_info->weapon_state = active_weapon->UpdateWeaponState(timestamp);
    m_player_info->magazine_left = active_weapon->AmmunitionLeft();
    m_player_info->laser_sight = (HoldingPickup() == false);

    m_player_info->cooldown_id = 0;
    m_player_info->cooldown_fraction = 1.0f;

    const bool is_allocated = m_damage_system->IsAllocated(m_entity_id);
    if(is_allocated)
    {
        const DamageRecord* player_damage_record = m_damage_system->GetDamageRecord(m_entity_id);
        m_player_info->health_fraction = float(player_damage_record->health) / float(player_damage_record->full_health);
    }

    const auto find_active_cooldown = [](float cooldown){
        return cooldown < 1.0f;
    };

    const auto cooldown_it = std::find_if(std::begin(m_active_cooldowns), std::end(m_active_cooldowns), find_active_cooldown);
    if(cooldown_it != std::end(m_active_cooldowns))
    {
        const int index = std::distance(std::begin(m_active_cooldowns), cooldown_it);
        m_player_info->cooldown_id = index;
        m_player_info->cooldown_fraction = m_active_cooldowns[index];
    }
}

void PlayerLogic::UpdateAnimation(float aim_direction, const math::Vector& world_position, const math::Vector& player_velocity)
{
    float anim_speed = 1.0f;
    int anim_id = m_idle_anim_id;

    const bool facing_left = (aim_direction > 0.0f);
    const float velocity_magnitude = math::Length(player_velocity);

    if(velocity_magnitude > 0.2f)
    {
        anim_id = m_run_anim_id;
        anim_speed = std::clamp(math::Scale01(velocity_magnitude, 0.0f, 5.0f), 0.5f, 10.0f);
        m_footsteps_effect->EmitFootStepsAt(world_position - math::Vector(0.0f, 0.15f));
    }

    if(m_accumulated_step_distance >= tweak_values::footstep_length)
    {
        m_footsteps_effect->EmitFootStepsAt(world_position - math::Vector(0.0f, 0.15f));
        m_running_sound->Play();
        m_accumulated_step_distance = 0.0f;
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

void PlayerLogic::UpdateWeaponAnimation(const mono::UpdateContext& update_context)
{
    const math::Vector aim_target_vector = math::VectorFromAngle(m_aim_target);
    const math::Vector aim_direction_vector = math::VectorFromAngle(m_aim_direction);
    const float delta_angle_between = math::AngleBetweenPoints(aim_target_vector, aim_direction_vector);

    math::simple_spring_damper_implicit(
        m_aim_direction, m_aim_velocity, m_aim_direction - delta_angle_between, 0.1f, update_context.delta_s);
    m_aim_direction = math::NormalizeAngle(m_aim_direction);

    mono::Sprite* weapon_sprite = m_sprite_system->GetSprite(m_weapon_entity);
    if(m_aim_direction > 0.0f)
        weapon_sprite->SetProperty(mono::SpriteProperty::FLIP_VERTICAL);
    else
        weapon_sprite->ClearProperty(mono::SpriteProperty::FLIP_VERTICAL);

    math::Matrix& weapon_transform = m_transform_system->GetTransform(m_weapon_entity);
    weapon_transform =
        math::CreateMatrixWithPosition(math::Vector(0.0f, -0.1f)) *
        math::CreateMatrixFromZRotation(m_aim_direction + math::PI_2()) *
        math::CreateMatrixWithPosition(math::Vector(0.1f, 0.0f));

    m_sprite_system->SetSpriteEnabled(m_weapon_entity, !HoldingPickup());
}

void PlayerLogic::ToDefault()
{ }

void PlayerLogic::DefaultState(const mono::UpdateContext& update_context)
{
    m_gamepad_controller.Update(update_context);

    const math::Vector& position = m_transform_system->GetWorldPosition(m_entity_id);
    const math::Vector aim_vector = math::VectorFromAngle(m_aim_direction); // * 0.5f;
    const math::Vector fire_position = position + (aim_vector * 0.5f);
    const math::Vector target_fire_position = position + (aim_vector * 100.0f);

    IWeaponPtr& active_weapon = m_weapons[m_weapon_index];

    if(m_fire)
    {
        m_player_info->weapon_state = active_weapon->Fire(fire_position, m_aim_direction, update_context.timestamp);

        if(m_player_info->weapon_state == WeaponState::OUT_OF_AMMO && m_player_info->auto_reload)
            Reload(update_context.timestamp);
    }
    else if(m_stop_fire)
    {
        active_weapon->StopFire(update_context.timestamp);
        m_stop_fire = false;
    }

    UpdateWeaponAnimation(update_context);
    UpdateAnimation(m_aim_direction, position, m_player_info->velocity);

    mono::PhysicsSpace* physics_space = m_physics_system->GetSpace();
    mono::QueryResult query_result = physics_space->QueryFirst(position, target_fire_position, PLAYER_BULLET_MASK);
    m_player_info->aim_target = (query_result.body != nullptr) ? query_result.point : math::InfVec;

    if(m_player_info->player_state == PlayerState::DEAD)
        m_state.TransitionTo(PlayerStates::DEAD);
}

void PlayerLogic::ToDead()
{
    if(m_death_anim_id != -1)
    {
        mono::ISprite* sprite = m_sprite_system->GetSprite(m_entity_id);
        sprite->SetAnimation(m_death_anim_id);
    }

    m_sprite_system->SetSpriteEnabled(m_entity_id, false);
    m_sprite_system->SetSpriteEnabled(m_weapon_entity, false);
    m_light_system->SetLightEnabled(m_entity_id, false);

    if(HoldingPickup())
        Throw(0.0f);
}

void PlayerLogic::DeadState(const mono::UpdateContext& update_context)
{
    if(m_player_info->player_state == PlayerState::ALIVE)
        m_state.TransitionTo(PlayerStates::DEFAULT);
}

void PlayerLogic::ExitDead()
{
    m_sprite_system->SetSpriteEnabled(m_entity_id, true);
    m_sprite_system->SetSpriteEnabled(m_weapon_entity, true);

    m_light_system->SetLightEnabled(m_entity_id, true);
}

void PlayerLogic::ToBlink()
{
    m_sprite_system->SetSpriteEnabled(m_entity_id, false);
    m_sprite_system->SetSpriteEnabled(m_weapon_entity, false);

    const math::Vector& position = m_transform_system->GetWorldPosition(m_entity_id);

    const mono::Entity decoy_entity = m_entity_system->CreateEntity("res/entities/wooden_log.entity");
    m_entity_system->AddComponent(decoy_entity.id, BEHAVIOUR_COMPONENT);
    DecoyLogic* decoy_logic = new DecoyLogic(decoy_entity.id, m_entity_system);
    m_logic_system->AddLogic(decoy_entity.id, decoy_logic);

    math::Matrix& decoy_transform = m_transform_system->GetTransform(decoy_entity.id);
    math::Position(decoy_transform, position);

    const float degree_offset = mono::Random(-10.0f, 10.0f);
    math::RotateZ(decoy_transform, math::ToRadians(degree_offset));

    m_transform_system->SetTransformState(decoy_entity.id, mono::TransformState::CLIENT);

    m_smoke_effect->EmitSmokeAt(position);
    m_blink_sound->Play();

    m_blink_duration_counter = 0.0f;
    m_blink_cooldown = 0.0f;
}

void PlayerLogic::BlinkState(const mono::UpdateContext& update_context)
{
    m_blink_duration_counter += update_context.delta_s;

    if(m_blink_duration_counter >= tweak_values::blink_duration_s)
    {
        mono::IBody* body = m_physics_system->GetBody(m_entity_id);

        const math::Vector new_position = body->GetPosition() + (m_blink_direction * tweak_values::blink_distance);
        body->SetPosition(new_position);
        m_state.TransitionTo(PlayerStates::DEFAULT);
    }
}

void PlayerLogic::ExitBlink()
{
    m_sprite_system->SetSpriteEnabled(m_entity_id, true);
    m_sprite_system->SetSpriteEnabled(m_weapon_entity, true);
}

void PlayerLogic::Fire()
{
    if(HoldingPickup())
        return;

    m_fire = true;
}

void PlayerLogic::StopFire()
{
    // Only stop fire if fire was true.
    m_stop_fire = m_fire;
    m_fire = false;
}

void PlayerLogic::Reload(uint32_t timestamp)
{
    IWeaponPtr& active_weapon = m_weapons[m_weapon_index];
    active_weapon->Reload(timestamp);
}

void PlayerLogic::UseItemSlot(ItemSlotIndex slot_index)
{
    const bool can_trigger_interaction = m_interaction_system->CanPlayerTriggerInteraction(m_entity_id);
    if(can_trigger_interaction)
    {

    }

    ItemSlot& item_slot = m_item_slots[slot_index];
}

void PlayerLogic::HandlePickup(PickupType type, int amount)
{
    switch(type)
    {
    case PickupType::AMMO:
    {
        IWeaponPtr& active_weapon = m_weapons[m_weapon_index];
        active_weapon->AddAmmunition(amount);
        break;
    }
    case PickupType::HEALTH:
    {
        m_damage_system->GainHealth(m_entity_id, amount);
        break;
    }
    case PickupType::SCORE:
        break;

    case PickupType::WEAPON_PISTOL:
    case PickupType::WEAPON_PLASMA:
    case PickupType::WEAPON_SHOTGUN:
        break;
    };
}

void PlayerLogic::SelectWeapon(WeaponSelection selection)
{
    const int modifier = (selection == WeaponSelection::Next) ? 1 : -1;
    m_weapon_index = std::clamp(m_weapon_index + modifier, 0, N_WEAPONS -1);
}

void PlayerLogic::Throw(float throw_force)
{
    if(!HoldingPickup())
        return;

    mono::IBody* body = m_physics_system->GetBody(m_picked_up_id);
    if(body)
    {
        m_physics_system->ReleaseConstraint(m_pickup_constraint);
        m_pickup_constraint = nullptr;

        const math::Vector throw_direction = math::Normalized(math::VectorFromAngle(m_aim_direction));
        body->ApplyLocalImpulse(throw_direction * throw_force, math::ZeroVec);

        const std::vector<mono::IShape*>& shapes = m_physics_system->GetShapesAttachedToBody(m_picked_up_id);
        for(mono::IShape* shape : shapes)
            shape->SetCollisionBit(CollisionCategory::PLAYER);
    }

    m_interaction_system->SetInteractionEnabled(m_picked_up_id, true);

    const PackageAction action = (throw_force > 0.0f) ? PackageAction::THROWN : PackageAction::DROPPED;
    m_event_handler->DispatchEvent(PackagePickupEvent(m_entity_id, m_picked_up_id, action));

    m_picked_up_id = mono::INVALID_ID;
}

void PlayerLogic::ThrowAction()
{
    Throw(100.0f);
}

void PlayerLogic::PickupDrop()
{
    if(HoldingPickup())
    {
        Throw(0.0f);
        return;
    }

    const InteractionCallback interaction_callback = [this](uint32_t interaction_id, InteractionType interaction_type) {
        if(interaction_type == InteractionType::PICKUP && m_picked_up_id == mono::INVALID_ID)
        {
            m_interaction_system->SetInteractionEnabled(interaction_id, false);
            m_picked_up_id = interaction_id;

            mono::IBody* player_body = m_physics_system->GetBody(m_entity_id);
            mono::IBody* pickup_body = m_physics_system->GetBody(m_picked_up_id);
            m_pickup_constraint = m_physics_system->CreateSlideJoint(player_body, pickup_body, math::ZeroVec, math::ZeroVec, 0.05f, 0.25f);

            const std::vector<mono::IShape*>& shapes = m_physics_system->GetShapesAttachedToBody(m_picked_up_id);
            for(mono::IShape* shape : shapes)
                shape->ClearCollisionBit(CollisionCategory::PLAYER);

            m_event_handler->DispatchEvent(PackagePickupEvent(m_entity_id, m_picked_up_id, PackageAction::PICKED_UP));
        }
    };
    m_interaction_system->TryTriggerInteraction(m_entity_id, interaction_callback);
}

bool PlayerLogic::HoldingPickup() const
{
    return (m_picked_up_id != mono::INVALID_ID);
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
    m_aim_target = aim_direction;
}

void PlayerLogic::Blink(const math::Vector& direction)
{
    if(m_blink_cooldown < tweak_values::blink_cooldown_threshold_s)
        return;

    m_blink_direction = direction;
    m_state.TransitionTo(PlayerStates::BLINK);
}

void PlayerLogic::Shockwave()
{
    if(m_shockwave_cooldown < tweak_values::shockwave_cooldown_s)
        return;

    game::ShockwaveAt(m_physics_system, m_player_info->position, 10.0f);
    m_shockwave_cooldown = 0;

    const math::Vector world_position = m_transform_system->GetWorldPosition(m_entity_id);
    m_shockwave_effect->EmittAt(world_position);
}

void PlayerLogic::Shield()
{
    if(m_shield_cooldown < tweak_values::shield_cooldown_s)
        return;

    m_shield_cooldown = 0;

}
