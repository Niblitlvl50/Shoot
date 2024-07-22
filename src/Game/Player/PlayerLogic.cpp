
#include "PlayerLogic.h"
#include "DecoyLogic.h"
#include "Player/PlayerInfo.h"

#include "Entity/EntityLogicSystem.h"
#include "Entity/Component.h"
#include "Events/PackageEvents.h"

#include "SystemContext.h"

#include "Debug/IDebugDrawer.h"
#include "Input/InputSystem.h"
#include "TransformSystem/TransformSystem.h"
#include "Physics/PhysicsSystem.h"
#include "Physics/IShape.h"
#include "Physics/IConstraint.h"
#include "Physics/PhysicsSpace.h"
#include "Rendering/RenderSystem.h"
#include "Rendering/Sprite/Sprite.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Sprite/SpriteProperties.h"
#include "Rendering/Lights/LightSystem.h"
#include "Particle/ParticleSystem.h"

#include "DamageSystem/DamageSystem.h"
#include "InteractionSystem/InteractionSystem.h"
#include "Weapons/IWeapon.h"
#include "Weapons/WeaponSystem.h"
#include "Weapons/Modifiers/DamageModifier.h"
#include "Entity/TargetSystem.h"

#include "EntitySystem/IEntityManager.h"
#include "EventHandler/EventHandler.h"
#include "Events/PauseEvent.h"
#include "Events/PlayerEvents.h"
#include "Math/MathFunctions.h"
#include "Math/CriticalDampedSpring.h"
#include "Util/Random.h"

#include "Effects/SmokeEffect.h"
#include "Effects/ShockwaveEffect.h"
#include "Effects/FootStepsEffect.h"
#include "Effects/WeaponModifierEffect.h"
#include "Pickups/PickupSystem.h"
#include "Shockwave.h"

#include "HookshotLogic.h"

#include <cmath>

namespace tweak_values
{
    constexpr float force_multiplier = 250.0f;
    constexpr float blink_duration_s = 0.2f;
    constexpr float blink_distance = 2.0f;
    constexpr float blink_cooldown_threshold_s = 2.0f;
    constexpr float shockwave_cooldown_s = 2.0f;
    constexpr float shield_cooldown_s = 2.0f;

    constexpr float footstep_length = 0.4f;
    constexpr float stamina_consumption_per_s = 0.75f;
    constexpr float stamina_recover_thresold_s = 0.75f;
}

using namespace game;

PlayerLogic::PlayerLogic(
    uint32_t entity_id,
    PlayerInfo* player_info,
    const PlayerConfig& config,
    mono::InputSystem* input_system,
    mono::EventHandler* event_handler,
    mono::SystemContext* system_context)
    : m_entity_id(entity_id)
    , m_player_info(player_info)
    , m_config(config)
    , m_gamepad_controller(this)
    , m_keyboard_controller(this)
    , m_event_handler(event_handler)
    , m_pause(false)
    , m_fire(false)
    , m_stop_fire(false)
    , m_aim_direction(0.0f)
    , m_aim_target(0.0f)
    , m_aim_velocity(0.0f)
    , m_sprint(false)
    , m_stamina(1.0f)
    , m_stamina_recover_timer_s(0.0f)
    , m_weapon_index(0)
    , m_accumulated_step_distance(0.0f)
    , m_blink_cooldown(tweak_values::blink_cooldown_threshold_s)
    , m_shockwave_cooldown(tweak_values::shockwave_cooldown_s)
    , m_shield_cooldown(tweak_values::shield_cooldown_s)
    , m_damage_modifier_handle(0)
    , m_picked_up_id(mono::INVALID_ID)
    , m_pickup_constraint(nullptr)
{
    m_transform_system = system_context->GetSystem<mono::TransformSystem>();
    m_input_system = system_context->GetSystem<mono::InputSystem>();
    m_physics_system = system_context->GetSystem<mono::PhysicsSystem>();
    m_render_system = system_context->GetSystem<mono::RenderSystem>();
    m_sprite_system = system_context->GetSystem<mono::SpriteSystem>();
    m_light_system = system_context->GetSystem<mono::LightSystem>();
    m_entity_system = system_context->GetSystem<mono::IEntityManager>();
    m_damage_system = system_context->GetSystem<DamageSystem>();
    m_pickup_system = system_context->GetSystem<PickupSystem>();
    m_interaction_system = system_context->GetSystem<InteractionSystem>();
    m_weapon_system = system_context->GetSystem<game::WeaponSystem>();
    m_logic_system = system_context->GetSystem<game::EntityLogicSystem>();
    m_target_system = system_context->GetSystem<game::TargetSystem>();

    m_input_context = m_input_system->CreateContext(1, mono::InputContextBehaviour::ConsumeIfHandled, "PlayerLogicInput");
    m_input_context->keyboard_input = &m_keyboard_controller;
    m_input_context->mouse_input = &m_keyboard_controller;
    m_input_context->controller_input = &m_gamepad_controller;
    m_input_context->controller_id = player_info->controller_id;

    mono::ISprite* sprite = m_sprite_system->GetSprite(entity_id);
    m_idle_anim_id = sprite->GetAnimationIdFromName("idle");
    m_run_anim_id = sprite->GetAnimationIdFromName("run");
    m_run_up_anim_id = sprite->GetAnimationIdFromName("run_up");
    if(m_run_up_anim_id == -1)
        m_run_up_anim_id = m_run_anim_id;
    m_death_anim_id = sprite->GetAnimationIdFromName("death");

    using namespace std::placeholders;
    game::PickupCallback handle_pickups = std::bind(&PlayerLogic::HandlePickup, this, _1, _2);
    m_pickup_system->RegisterPickupTarget(m_entity_id, handle_pickups);

    m_switch_weapon_sound = audio::CreateSound("res/sound/weapon_switch.wav", audio::SoundPlayback::ONCE);
    m_blink_sound = audio::CreateSound("res/sound/punch.wav", audio::SoundPlayback::ONCE);
    m_running_sound = audio::CreateSound("res/sound/running-cartoon-footstep_1.wav", audio::SoundPlayback::ONCE);
    m_running_sound->SetVolume(0.5f);

    mono::ParticleSystem* particle_system = system_context->GetSystem<mono::ParticleSystem>();
    m_smoke_effect = std::make_unique<SmokeEffect>(particle_system, m_entity_system);
    m_shockwave_effect = std::make_unique<ShockwaveEffect>(m_transform_system, particle_system, m_entity_system);
    m_footsteps_effect = std::make_unique<FootStepsEffect>(particle_system, m_entity_system);
    m_weapon_modifier_effect = std::make_unique<WeaponModifierEffect>(m_transform_system, particle_system, m_entity_system);

    const mono::Entity spawned_weapon = m_entity_system->SpawnEntity(m_config.weapon_entity.c_str());
    m_weapon_entity = spawned_weapon.id;
    m_transform_system->ChildTransform(m_weapon_entity, m_entity_id);

    m_weapon_modifier_effect->AttachToEntityWithOffset(m_weapon_entity, math::Vector(0.2f, 0.0f));

    // Make sure we have a weapon
    m_weapons[0] = m_weapon_system->CreatePrimaryWeapon(entity_id, WeaponFaction::PLAYER);
    m_weapons[1] = m_weapon_system->CreateSecondaryWeapon(entity_id, WeaponFaction::PLAYER);
    m_weapons[2] = m_weapon_system->CreateTertiaryWeapon(entity_id, WeaponFaction::PLAYER);

    m_aim_target = m_aim_direction = -math::PI_2();

    const PlayerStateMachine::StateTable state_table = {
        PlayerStateMachine::MakeState(PlayerStates::DEFAULT, &PlayerLogic::ToDefault, &PlayerLogic::DefaultState, this),
        PlayerStateMachine::MakeState(PlayerStates::DEAD, &PlayerLogic::ToDead, &PlayerLogic::DeadState, &PlayerLogic::ExitDead, this),
        PlayerStateMachine::MakeState(PlayerStates::BLINK, &PlayerLogic::ToBlink, &PlayerLogic::BlinkState, &PlayerLogic::ExitBlink, this),
    };
    m_state.SetStateTableAndState(state_table, PlayerStates::DEFAULT);

    m_hookshot = std::make_unique<Hookshot>(m_entity_id, m_entity_system, m_physics_system, m_sprite_system, m_transform_system, m_logic_system);

    // Make sure the player info is updated when constructed.
    UpdatePlayerInfo(0);
}

PlayerLogic::~PlayerLogic()
{
    m_input_system->ReleaseContext(m_input_context);

    m_pickup_system->UnregisterPickupTarget(m_entity_id);
    m_entity_system->ReleaseEntity(m_weapon_entity);
}

void PlayerLogic::DrawDebugInfo(IDebugDrawer* debug_drawer) const
{
    const math::Vector world_position = m_transform_system->GetWorldPosition(m_entity_id);

    char buffer[512] = {};
    std::snprintf(buffer, std::size(buffer), "%.2f, %s", m_stamina, m_sprint ? "sprint" : "walk");
    debug_drawer->DrawWorldText(buffer, world_position, mono::Color::OFF_WHITE);
}

const char* PlayerLogic::GetDebugCategory() const
{
    return "player";
}

void PlayerLogic::Update(const mono::UpdateContext& update_context)
{
    m_state.UpdateState(update_context);

    // This needs to be setup later since in the contructor the familiar is not created yet. :/ 
    if(!m_familiar_weapon)
        m_familiar_weapon = m_weapon_system->CreatePrimaryWeapon(m_player_info->familiar_entity_id, WeaponFaction::PLAYER);

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

    float stamina_multiplier;
    if(m_sprint)
    {
        stamina_multiplier = 1.0f;
    }
    else if(m_stamina_recover_timer_s < tweak_values::stamina_recover_thresold_s)
    {
        m_stamina_recover_timer_s += update_context.delta_s;
        stamina_multiplier = 0.0f;
    }
    else
    {
        stamina_multiplier = -1.0f;
    }

    m_stamina = std::clamp(m_stamina - (update_context.delta_s * tweak_values::stamina_consumption_per_s * stamina_multiplier), 0.0f, 1.0f);

    UpdateAutoAim();
    UpdatePlayerInfo(update_context.timestamp);
}

void PlayerLogic::UpdateAutoAim()
{
    if(!m_player_info->persistent_data.auto_aim)
        return;

    mono::PhysicsSpace* space = m_physics_system->GetSpace();

    const mono::QueryFilter filter_visible = [this, space](uint32_t entity_id, const math::Vector& point) {
        const uint32_t query_category = CollisionCategory::ENEMY | CollisionCategory::STATIC;
        const mono::QueryResult result = space->QueryFirst(m_player_info->position, point, query_category);
        return (result.collision_category & CollisionCategory::ENEMY);
    };

    mono::QueryResult query_result = space->QueryNearest(m_player_info->position, 3.0f, CollisionCategory::ENEMY, filter_visible);
    if(query_result.body)
    {
        const math::Vector delta_to_target = (query_result.body->GetPosition() - m_player_info->position);
        const float aim_direction = math::AngleFromVector(delta_to_target);
        SetAimDirection(aim_direction);
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
    m_player_info->aim_crosshair_screen_position = m_aim_screen_position;

    IWeaponPtr& active_weapon = m_weapons[m_weapon_index];

    m_player_info->weapon_type = active_weapon->GetWeaponSetup();
    m_player_info->weapon_state = active_weapon->UpdateWeaponState(timestamp);
    m_player_info->magazine_left = active_weapon->AmmunitionLeft();
    m_player_info->laser_sight = true;

    m_player_info->cooldown_id = 0;
    m_player_info->cooldown_fraction = 1.0f;

    const bool is_allocated = m_damage_system->IsAllocated(m_entity_id);
    if(is_allocated)
    {
        const DamageRecord* player_damage_record = m_damage_system->GetDamageRecord(m_entity_id);
        m_player_info->health_fraction = float(player_damage_record->health) / float(player_damage_record->full_health);
    }

    m_player_info->stamina_fraction = m_stamina;

    const float experience_fraction = m_player_info->persistent_data.experience / 100.0f;
    m_player_info->experience_fraction = experience_fraction;

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

    const float damage_modifier_fraction = m_weapon_system->GetDurationFractionForModifierOnEntity(m_entity_id, m_damage_modifier_handle);
    m_player_info->powerup_fraction = damage_modifier_fraction;

    m_player_info->last_used_input = m_input_context->most_recent_input;
}

void PlayerLogic::UpdateMovement(const mono::UpdateContext& update_context)
{
    const float length_squared = math::LengthSquared(m_movement_direction);
    if(length_squared <= FLT_EPSILON)
    {

    }
    else
    {
        const float sprint_multiplier = m_sprint && HasStamina() ? 1.5f : 1.0f;
        ApplyForce(m_movement_direction * tweak_values::force_multiplier * sprint_multiplier);
    }
}

void PlayerLogic::UpdateAnimation(const mono::UpdateContext& update_context, float aim_direction, const math::Vector& world_position, const math::Vector& player_velocity)
{
    float anim_speed = 1.0f;
    int anim_id = m_idle_anim_id;

    const float velocity_magnitude = math::Length(player_velocity);

    const bool facing_left = (aim_direction > 0.0f);
    const bool facing_down = (std::abs(player_velocity.x) >= player_velocity.y);

    if(velocity_magnitude > 0.2f)
    {
        anim_id = facing_down ? m_run_anim_id : m_run_up_anim_id;
        anim_speed = std::clamp(math::Scale01(velocity_magnitude, 0.0f, 3.0f), 0.5f, 10.0f);
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

    const bool reverse_playback = (facing_left && player_velocity.x > 0.0f) || (!facing_left && player_velocity.x < 0.0f);
    if(reverse_playback)
        sprite->SetAnimationPlayback(mono::PlaybackMode::PLAYING_REVERSE);
    else
        sprite->SetAnimationPlayback(mono::PlaybackMode::PLAYING);


    // Weapon animation

    mono::Sprite* weapon_sprite = m_sprite_system->GetSprite(m_weapon_entity);

    const float weapon_sort_offset = facing_down ? -0.1 : 0.1f;
    m_render_system->UpdateLayer(m_weapon_entity, 0, weapon_sort_offset);

    IWeaponPtr& active_weapon = m_weapons[m_weapon_index];
    const game::WeaponSetup& weapon_setup = active_weapon->GetWeaponSetup();

    if(m_player_info->weapon_type != weapon_setup)
    {
        const WeaponBulletCombination& weapon_bullet = m_weapon_system->GetWeaponBulletConfigForHash(weapon_setup.weapon_identifier_hash);
        m_sprite_system->SetSpriteFile(m_weapon_entity, weapon_bullet.sprite_file.c_str());
    }

    const math::Vector aim_target_vector = math::VectorFromAngle(m_aim_target);
    const math::Vector aim_direction_vector = math::VectorFromAngle(m_aim_direction);
    const float delta_angle_between = math::AngleBetweenPoints(aim_target_vector, aim_direction_vector);

    math::simple_spring_damper_implicit(
        m_aim_direction, m_aim_velocity, m_aim_direction - delta_angle_between, 0.1f, update_context.delta_s);
    m_aim_direction = math::NormalizeAngle(m_aim_direction);
    if(m_aim_direction > 0.0f)
        weapon_sprite->SetProperty(mono::SpriteProperty::FLIP_VERTICAL);
    else
        weapon_sprite->ClearProperty(mono::SpriteProperty::FLIP_VERTICAL);

    math::Matrix& weapon_transform = m_transform_system->GetTransform(m_weapon_entity);
    weapon_transform =
        math::CreateMatrixWithPosition(math::Vector(0.0f, -0.075f)) *
        math::CreateMatrixFromZRotation(m_aim_direction + math::PI_2()) *
        math::CreateMatrixWithPosition(math::Vector(0.1f, 0.0f));
}

void PlayerLogic::UpdateController(const mono::UpdateContext& update_context)
{
    ResetMovement();

    const uint32_t player_index = FindPlayerIndex(m_player_info);

    // Select most recent input if player zero, else just go with gamepad. 
    if(m_input_context->most_recent_input == mono::InputContextType::Controller || player_index > 0)
        m_gamepad_controller.Update(update_context);
    else
        m_keyboard_controller.Update(update_context);
}

void PlayerLogic::ToDefault()
{ }

void PlayerLogic::DefaultState(const mono::UpdateContext& update_context)
{
    UpdateController(update_context);

    const math::Vector& position = m_transform_system->GetWorldPosition(m_entity_id);
    const math::Vector aim_vector = math::VectorFromAngle(m_aim_direction);
    const math::Vector fire_position = position + (aim_vector * 0.5f);
    const math::Vector target_fire_position = position + (aim_vector * 100.0f);

    uint32_t collision_mask = PLAYER_BULLET_MASK;
    if(HoldingPickup())
        collision_mask &= ~CollisionCategory::PACKAGE;

    mono::PhysicsSpace* physics_space = m_physics_system->GetSpace();
    mono::QueryResult query_result = physics_space->QueryFirst(position, target_fire_position, collision_mask);
    m_player_info->aim_target = (query_result.body != nullptr) ? query_result.point : target_fire_position;

    IWeaponPtr& active_weapon = m_weapons[m_weapon_index];
    if(m_fire && !m_sprint)
    {
        m_player_info->weapon_state = active_weapon->Fire(fire_position, m_player_info->aim_target, update_context.timestamp);

        if(m_player_info->weapon_state == WeaponState::OUT_OF_AMMO && m_player_info->persistent_data.auto_reload)
            Reload(update_context.timestamp);

        const math::Vector familiar_position = m_transform_system->GetWorldPosition(m_player_info->familiar_entity_id);
        m_familiar_weapon->Fire(familiar_position, m_player_info->aim_target, update_context.timestamp);
    }
    else if(m_stop_fire)
    {
        active_weapon->StopFire(update_context.timestamp);
        m_familiar_weapon->StopFire(update_context.timestamp);
        m_stop_fire = false;
    }

    UpdateMovement(update_context);
    UpdateAnimation(update_context, m_aim_direction, position, m_player_info->velocity);

    m_hookshot->Update(update_context);

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
    m_target_system->SetTargetEnabled(m_entity_id, false);

    if(HoldingPickup())
        Throw(0.0f);
}

void PlayerLogic::DeadState(const mono::UpdateContext& update_context)
{
    UpdateController(update_context);

    if(m_player_info->player_state == PlayerState::ALIVE)
        m_state.TransitionTo(PlayerStates::DEFAULT);
}

void PlayerLogic::ExitDead()
{
    m_sprite_system->SetSpriteEnabled(m_entity_id, true);
    m_sprite_system->SetSpriteEnabled(m_weapon_entity, true);
    m_light_system->SetLightEnabled(m_entity_id, true);
    m_target_system->SetTargetEnabled(m_entity_id, true);
}

void PlayerLogic::ToBlink()
{
    m_sprite_system->SetSpriteEnabled(m_entity_id, false);
    m_sprite_system->SetSpriteEnabled(m_weapon_entity, false);
    m_target_system->SetTargetEnabled(m_entity_id, false);

    const math::Vector& position = m_transform_system->GetWorldPosition(m_entity_id);

    const mono::Entity decoy_entity = m_entity_system->SpawnEntity(m_config.decoy_entity.c_str());
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
        const math::Vector target_position = body->GetPosition() + (m_blink_direction * tweak_values::blink_distance);

        mono::PhysicsSpace* space = m_physics_system->GetSpace();
        const mono::QueryResult result = space->QueryFirst(body->GetPosition(), target_position, CollisionCategory::STATIC);
        const math::Vector new_position = (result.body != nullptr) ? result.point : target_position;
        body->SetPosition(new_position);

        m_state.TransitionTo(PlayerStates::DEFAULT);
    }
}

void PlayerLogic::ExitBlink()
{
    m_sprite_system->SetSpriteEnabled(m_entity_id, true);
    m_sprite_system->SetSpriteEnabled(m_weapon_entity, true);
    m_target_system->SetTargetEnabled(m_entity_id, true);
}

void PlayerLogic::Fire(uint32_t timestamp)
{
    IWeaponPtr& active_weapon = m_weapons[m_weapon_index];
    const WeaponState state = active_weapon->GetWeaponState();

    if(!m_fire && state == WeaponState::OUT_OF_AMMO)
        active_weapon->Reload(timestamp);

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

    //ItemSlot& item_slot = m_item_slots[slot_index];
}

void PlayerLogic::HandlePickup(PickupType type, int meta_data)
{
    switch(type)
    {
    case PickupType::AMMO:
    {
        IWeaponPtr& active_weapon = m_weapons[m_weapon_index];
        active_weapon->AddAmmunition(active_weapon->MagazineSize());
        break;
    }
    case PickupType::HEALTH:
    {
        DamageRecord* damage_record = m_damage_system->GetDamageRecord(m_entity_id);
        damage_record->health = damage_record->full_health;
        break;
    }
    case PickupType::SECOND_WIND:
    {
        m_blink_cooldown = tweak_values::blink_cooldown_threshold_s;
        m_shockwave_cooldown = tweak_values::shockwave_cooldown_s;
        m_shield_cooldown = tweak_values::shield_cooldown_s;
        break;
    }
    case PickupType::COINS:
    {
        m_player_info->persistent_data.chips += meta_data;
        break;
    }
    case PickupType::EXPERIENCE:
    {
        m_player_info->persistent_data.experience++;

        AddBulletWallBuff(meta_data);
        break;
    }
    case PickupType::DAMAGE_BUFF:
    {
        AddDamageBuff(meta_data);
        break;
    }
    };
}

void PlayerLogic::CycleWeapon()
{
    m_weapon_index++;
    if(m_weapon_index == N_WEAPONS)
        m_weapon_index = 0;

    m_switch_weapon_sound->Play();
}

void PlayerLogic::AddDamageBuff(int meta_data)
{
    constexpr float duration_s = 10.0f;
    m_damage_modifier_handle = m_weapon_system->AddModifierForIdWithDuration(m_entity_id, duration_s, new DamageModifier(2.0f));
    m_weapon_modifier_effect->EmitForDuration(duration_s);

    m_player_info->powerup_id = WeaponModifier::DAMAGE;
}

void PlayerLogic::AddBulletWallBuff(int meta_data)
{
    constexpr float duration_s = 10.0f;
    m_damage_modifier_handle = m_weapon_system->AddModifierForIdWithDuration(m_entity_id, duration_s, new BulletWallModifier());
    m_weapon_modifier_effect->EmitForDuration(duration_s);

    m_player_info->powerup_id = WeaponModifier::SPREAD;
}

void PlayerLogic::TriggerHookshot()
{
    const math::Vector& position = m_transform_system->GetWorldPosition(m_entity_id);
    m_hookshot->TriggerHookshot(position, m_aim_direction);
}

void PlayerLogic::ReleaseHookshot()
{
    m_hookshot->DetachHookshot();
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

        body->SetMass(m_pickup_mass);

        const math::Vector throw_direction = math::Normalized(math::VectorFromAngle(m_aim_direction));
        body->ApplyLocalImpulse(throw_direction * throw_force, math::ZeroVec);

        const std::vector<mono::IShape*>& shapes = m_physics_system->GetShapesAttachedToBody(m_picked_up_id);
        for(mono::IShape* shape : shapes)
            shape->SetCollisionBit(CollisionCategory::PLAYER | CollisionCategory::PLAYER_BULLET);
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
            const bool has_body = m_physics_system->IsAllocated(interaction_id);
            if(!has_body)
                return;

            m_picked_up_id = interaction_id;
            m_interaction_system->SetInteractionEnabled(interaction_id, false);

            mono::IBody* player_body = m_physics_system->GetBody(m_entity_id);
            mono::IBody* pickup_body = m_physics_system->GetBody(m_picked_up_id);
            m_pickup_constraint = m_physics_system->CreateSlideJoint(player_body, pickup_body, math::ZeroVec, math::ZeroVec, 0.05f, 0.35f);

            const std::vector<mono::IShape*>& shapes = m_physics_system->GetShapesAttachedToBody(m_picked_up_id);
            for(mono::IShape* shape : shapes)
                shape->ClearCollisionBit(CollisionCategory::PLAYER | CollisionCategory::PLAYER_BULLET);

            m_pickup_mass = pickup_body->GetMass();
            pickup_body->SetMass(0.1f);

            m_event_handler->DispatchEvent(PackagePickupEvent(m_entity_id, m_picked_up_id, PackageAction::PICKED_UP));


            // Must handle destroyed package while holding it and then player death.

            //const mono::ReleaseCallback release_callback = [this](uint32_t entity_id) {
            //};
            //const uint32_t m_package_release_callback = m_entity_system->AddReleaseCallback(m_picked_up_id, release_callback);
        }
        else if(interaction_type == InteractionType::WEAPON)
        {
            {
                // Drop current weapon as a pickup
                const IWeaponPtr& current_weapon = m_weapons[m_weapon_index];
                const game::WeaponSetup weapon_setup = current_weapon->GetWeaponSetup();
                const math::Matrix& interaction_transform = m_transform_system->GetTransform(interaction_id);
                m_weapon_system->SpawnWeaponPickupAt(weapon_setup, math::GetPosition(interaction_transform) + math::Vector(2.0f, 0.0));
            }

            m_weapons[m_weapon_index] = m_weapon_system->CreatePrimaryWeapon(interaction_id, WeaponFaction::PLAYER);
        }
    };
    m_interaction_system->TryTriggerInteraction(m_entity_id, interaction_callback);
}

bool PlayerLogic::HoldingPickup() const
{
    return (m_picked_up_id != mono::INVALID_ID);
}

void PlayerLogic::Sprint()
{
    m_sprint = true;
    m_stamina_recover_timer_s = 0.0f;
}

void PlayerLogic::StopSprint()
{
    m_sprint = false;
}

bool PlayerLogic::HasStamina() const
{
    return m_stamina > 0.0f;
}

void PlayerLogic::MoveInDirection(const math::Vector& direction)
{
    m_movement_direction = direction;
}

void PlayerLogic::ResetMovement()
{
    m_movement_direction = math::ZeroVec;

    mono::IBody* body = m_physics_system->GetBody(m_entity_id);
    body->ResetForces();
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

void PlayerLogic::SetAimDirection(float aim_direction)
{
    m_aim_target = aim_direction;
}

void PlayerLogic::SetAimScreenPosition(const math::Vector& aim_screen_position)
{
    m_aim_screen_position = aim_screen_position;
}

void PlayerLogic::Blink(const math::Vector& direction)
{
    if(m_blink_cooldown < tweak_values::blink_cooldown_threshold_s)
        return;

    const float length_sq = math::LengthSquared(direction);
    m_blink_direction = (length_sq > FLT_EPSILON) ? direction : math::VectorFromAngle(m_aim_direction);

    m_state.TransitionTo(PlayerStates::BLINK);
}

void PlayerLogic::Shockwave()
{
    /*
    if(m_shockwave_cooldown < tweak_values::shockwave_cooldown_s)
        return;

    game::ShockwaveAt(m_physics_system, m_player_info->position, 10.0f);
    m_shockwave_cooldown = 0;

    const math::Vector world_position = m_transform_system->GetWorldPosition(m_entity_id);
    m_shockwave_effect->EmittAt(world_position);
    */
}

void PlayerLogic::Shield()
{
    if(m_shield_cooldown < tweak_values::shield_cooldown_s)
        return;

    m_shield_cooldown = 0;
}

void PlayerLogic::RespawnPlayer()
{
    m_event_handler->DispatchEvent(game::RespawnPlayerEvent(m_entity_id));
}

void PlayerLogic::TogglePauseGame()
{
    m_pause = !m_pause;
    m_event_handler->DispatchEvent(event::PauseEvent(m_pause));
}

