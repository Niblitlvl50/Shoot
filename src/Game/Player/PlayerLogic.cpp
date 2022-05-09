
#include "PlayerLogic.h"
#include "DecoyLogic.h"
#include "Player/PlayerInfo.h"

#include "Entity/EntityLogicSystem.h"
#include "Entity/Component.h"

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
#include "Factories.h"
#include "Weapons/IWeapon.h"
#include "Weapons/IWeaponFactory.h"

#include "EntitySystem/IEntityManager.h"
#include "Math/MathFunctions.h"
#include "Util/Random.h"

#include "Effects/SmokeEffect.h"
#include "Effects/ShockwaveEffect.h"
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
    , m_stop_fire(false)
    , m_aim_direction(0.0f)
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
    m_logic_system = system_context->GetSystem<game::EntityLogicSystem>();

    mono::ISprite* sprite = m_sprite_system->GetSprite(entity_id);
    m_idle_anim_id = sprite->GetAnimationIdFromName("idle");
    m_run_anim_id = sprite->GetAnimationIdFromName("run");

    using namespace std::placeholders;
    game::PickupCallback handle_pickups = std::bind(&PlayerLogic::HandlePickup, this, _1, _2);
    m_pickup_system->RegisterPickupTarget(m_entity_id, handle_pickups);

    mono::ParticleSystem* particle_system = system_context->GetSystem<mono::ParticleSystem>();
    m_blink_sound = audio::CreateSound("res/sound/punch.wav", audio::SoundPlayback::ONCE);

    m_smoke_effect = std::make_unique<SmokeEffect>(particle_system, m_entity_system);
    m_shockwave_effect = std::make_unique<ShockwaveEffect>(m_transform_system, particle_system, m_entity_system);

    const mono::Entity spawned_weapon = m_entity_system->CreateEntity("res/entities/player_weapon.entity");
    m_weapon_entity = spawned_weapon.id;

    m_transform_system->ChildTransform(m_weapon_entity, m_entity_id);

    // Make sure we have a weapon
    SelectWeapon(game::PLASMA_GUN);
    m_aim_target = m_aim_direction = -math::PI_2();

    const PlayerStateMachine::StateTable state_table = {
        PlayerStateMachine::MakeState(PlayerStates::DEFAULT, &PlayerLogic::ToDefault, &PlayerLogic::DefaultState, this),
        PlayerStateMachine::MakeState(PlayerStates::DEAD, &PlayerLogic::ToDead, &PlayerLogic::DeadState, &PlayerLogic::ExitDead, this),
        PlayerStateMachine::MakeState(PlayerStates::BLINK, &PlayerLogic::ToBlink, &PlayerLogic::BlinkState, &PlayerLogic::ExitBlink, this),
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

    m_blink_cooldown += update_context.delta_s;
    m_shockwave_cooldown += update_context.delta_s;
    m_shield_cooldown += update_context.delta_s;

    m_active_cooldowns[PlayerAbility::WEAPON_RELOAD] = float(m_weapon->ReloadPercentage()) / 100.0f;
    m_active_cooldowns[PlayerAbility::BLINK] = m_blink_cooldown / tweak_values::blink_cooldown_threshold_s;
    m_active_cooldowns[PlayerAbility::SHOCKWAVE] = m_shockwave_cooldown / tweak_values::shockwave_cooldown_s;
    m_active_cooldowns[PlayerAbility::SHIELD] = m_shield_cooldown / tweak_values::shield_cooldown_s;
    m_active_cooldowns[PlayerAbility::WEAPON_AMMUNITION] =
        float(m_weapon->AmmunitionLeft()) / float(m_weapon->MagazineSize());

    UpdatePlayerInfo(update_context.timestamp);
}

void PlayerLogic::UpdatePlayerInfo(uint32_t timestamp)
{
    const math::Matrix& transform = m_transform_system->GetWorld(m_entity_id);
    mono::IBody* body = m_physics_system->GetBody(m_entity_id);

    m_player_info->position = math::GetPosition(transform);
    m_player_info->velocity = body->GetVelocity();
    m_player_info->direction = math::GetZRotation(transform);
    m_player_info->aim_direction = math::VectorFromAngle(m_aim_direction);

    m_player_info->weapon_type = m_weapon_type;
    m_player_info->weapon_state = m_weapon->UpdateWeaponState(timestamp);
    m_player_info->magazine_left = m_weapon->AmmunitionLeft();
    m_player_info->laser_sight = (HoldingPickup() == false);

    m_player_info->cooldown_id = 0;
    m_player_info->cooldown_fraction = 1.0f;


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

void PlayerLogic::UpdateAnimation(float aim_direction, const math::Vector& player_velocity)
{
    float anim_speed = 1.0f;
    int anim_id = m_idle_anim_id;

    const bool facing_left = (aim_direction > 0.0f);
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

void PlayerLogic::UpdateWeaponAnimation(const mono::UpdateContext& update_context)
{
    const math::Vector target_vector = math::VectorFromAngle(m_aim_target);
    const math::Vector direction_vector = math::VectorFromAngle(m_aim_direction);

    const float cross_value = math::Cross(target_vector, direction_vector);

    if(!math::IsPrettyMuchEquals(cross_value, 0.0f))
    {
        const bool rotate_clockwise = (cross_value < 0.0f);
        const float multiplier = rotate_clockwise ? 1.0f : -1.0f;
        m_aim_direction += (multiplier * update_context.delta_s * math::ToRadians(360.0f));

        {
            // Need something better here to determine if we should clamp or not. Perhaps cross again.
            const bool go_right_and_more = (rotate_clockwise && m_aim_direction > m_aim_target);
            const bool go_left_and_less = (!rotate_clockwise && m_aim_direction < m_aim_target);
            if(go_right_and_more || go_left_and_less)
                m_aim_direction = m_aim_target;
        }
    }

    mono::Sprite* weapon_sprite = m_sprite_system->GetSprite(m_weapon_entity);
    if(m_aim_direction > 0.0f)
        weapon_sprite->SetProperty(mono::SpriteProperty::FLIP_VERTICAL);
    else
        weapon_sprite->ClearProperty(mono::SpriteProperty::FLIP_VERTICAL);

    math::Matrix& weapon_transform = m_transform_system->GetTransform(m_weapon_entity);
    weapon_transform =
        math::CreateMatrixWithPosition(math::Vector(0.0f, -0.1f)) *
        math::CreateMatrixFromZRotation(m_aim_direction + math::PI_2()) *
        math::CreateMatrixWithPosition(math::Vector(0.2f, 0.0f));

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

    if(m_fire)
    {
        m_player_info->weapon_state = m_weapon->Fire(fire_position, m_aim_direction, update_context.timestamp);
    }
    else if(m_stop_fire)
    {
        m_weapon->StopFire(update_context.timestamp);
        m_stop_fire = false;
    }

    UpdateWeaponAnimation(update_context);
    UpdateAnimation(m_aim_direction, m_player_info->velocity);

    mono::PhysicsSpace* physics_space = m_physics_system->GetSpace();
    mono::QueryResult query_result = physics_space->QueryFirst(fire_position, target_fire_position, PLAYER_BULLET_MASK);
    m_player_info->aim_target = (query_result.body != nullptr) ? query_result.point : math::InfVec;

    if(m_player_info->player_state == PlayerState::DEAD)
        m_state.TransitionTo(PlayerStates::DEAD);
}

void PlayerLogic::ToDead()
{
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

void PlayerLogic::HandlePickup(PickupType type, int amount)
{
    switch(type)
    {
    case PickupType::AMMO:
        m_weapon->AddAmmunition(amount);
        break;
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
        HandleWeaponPickup(type);
        break;
    };
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
        }
    };
    m_interaction_system->TryTriggerInteraction(m_entity_id, interaction_callback);
}

bool PlayerLogic::HoldingPickup() const
{
    return (m_picked_up_id != mono::INVALID_ID);
}

void PlayerLogic::TriggerInteraction()
{
    System::Log("playerlogic|Trigger Interaction");
}

void PlayerLogic::SelectWeapon(WeaponSetup weapon)
{
    m_weapon = g_weapon_factory->CreateWeapon(weapon, WeaponFaction::PLAYER, m_entity_id);
    m_weapon_type = weapon;
}

void PlayerLogic::HandleWeaponPickup(PickupType type)
{
    static const std::unordered_map<PickupType, game::WeaponSetup> g_pickup_to_weapon = {
        { PickupType::WEAPON_PISTOL,    game::GENERIC },
        { PickupType::WEAPON_PLASMA,    game::PLASMA_GUN },
        { PickupType::WEAPON_SHOTGUN,   game::FLAK_CANON },
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
