
#include "TrainLogic.h"
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
#include "Weapons/Modifiers/BulletBehaviourModifiers.h"
#include "Entity/TargetSystem.h"
#include "Behaviour/PathFollowerSystem.h"
#include "RailwaySystem/RailwaySystem.h"

#include "EntitySystem/IEntityManager.h"
#include "EventHandler/EventHandler.h"
#include "Events/PauseEvent.h"
#include "Events/PlayerEvents.h"
#include "Math/MathFunctions.h"
#include "Math/CriticalDampedSpring.h"
#include "Util/Random.h"

#include "Effects/TrainSmokeEffect.h"
#include "Effects/WheelGrindEffect.h"
#include "Effects/ShockwaveEffect.h"
#include "Effects/FootStepsEffect.h"
#include "Effects/WeaponModifierEffect.h"
#include "Pickups/PickupSystem.h"
#include "DamageSystem/Shockwave.h"

#include "HookshotLogic.h"

#include <cmath>

namespace tweak_values
{
    constexpr float steam_sound_max_speed = 8.0f;
    constexpr float steam_sound_idle_pitch = 0.6f;
    constexpr float steam_sound_max_pitch = 1.6f;
    constexpr float steam_sound_pitch_halflife = 0.4f;

    constexpr float smoke_emit_interval_s = 0.4f;

    // speed (m/s) * curvature (rad/m) is roughly the train's angular velocity while
    // cornering - above this, the wheels are considered to be grinding against the rails.
    constexpr float grind_cornering_threshold = 1.2f;
    constexpr float grind_emit_interval_s = 0.08f;
}

namespace
{
    struct PlayerLevelExperience
    {
        int level;
        int current_level_experience;
        int next_level_experience;
    };
    
    PlayerLevelExperience GetPlayerLevelExperience(int player_experience, const std::vector<int>& player_levels)
    {
        auto it = std::lower_bound(player_levels.begin(), player_levels.end(), player_experience);
        const int level_index = std::distance(player_levels.begin(), it);
        
        PlayerLevelExperience weapon_level_exp;
        weapon_level_exp.level = level_index;
        weapon_level_exp.next_level_experience = *it;
        weapon_level_exp.current_level_experience = (it == player_levels.begin()) ? 0 : *(--it);
        
        return weapon_level_exp;
    }
}

using namespace game;

TrainLogic::TrainLogic(
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
//    , m_keyboard_controller(this)
    , m_event_handler(event_handler)
    , m_pause(false)
    , m_aim_direction(0.0f)
    , m_aim_target(0.0f)
    , m_aim_velocity(0.0f)
    , m_steam_pitch(tweak_values::steam_sound_idle_pitch)
    , m_steam_pitch_velocity(0.0f)
    , m_smoke_timer_s(0.0f)
    , m_grind_timer_s(0.0f)
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
    m_logic_system = system_context->GetSystem<game::EntityLogicSystem>();
    m_target_system = system_context->GetSystem<game::TargetSystem>();
    m_path_follower_system = system_context->GetSystem<game::PathFollowerSystem>();
    m_railway_system = system_context->GetSystem<game::RailwaySystem>();

    const System::ControllerId controller_id = player_info->controller_id;

    m_input_context = m_input_system->CreateContext(1, mono::InputContextBehaviour::ConsumeIfHandled, "PlayerLogicInput");
    //m_input_context->keyboard_input = (controller_id == System::ControllerId::Primary) ? &m_keyboard_controller : nullptr;
    //m_input_context->mouse_input = (controller_id == System::ControllerId::Primary) ? &m_keyboard_controller : nullptr;
    m_input_context->controller_input = &m_gamepad_controller;
    m_input_context->controller_id = controller_id;

    mono::ISprite* sprite = m_sprite_system->GetSprite(entity_id);
    m_idle_anim_id = sprite->GetAnimationIdFromName("idle");
    m_run_anim_id = sprite->GetAnimationIdFromName("run");
    m_run_up_anim_id = sprite->GetAnimationIdFromName("run_up");
    if(m_run_up_anim_id == -1)
        m_run_up_anim_id = m_run_anim_id;
    m_death_anim_id = sprite->GetAnimationIdFromName("death");

    using namespace std::placeholders;
    game::PickupCallback handle_pickups = std::bind(&TrainLogic::HandlePickup, this, _1, _2);
    m_pickup_system->RegisterPickupTarget(m_entity_id, handle_pickups);

    m_drop_box_sound = audio::CreateSound(
        "res/sound/punch.wav", audio::SoundPlayback::ONCE, audio::SoundSpatiality::NONE);
    m_pickup_box_sound = audio::CreateSound(
        "res/sound/punch.wav", audio::SoundPlayback::ONCE, audio::SoundSpatiality::NONE);
    
    m_horn_sound = audio::CreateSound(
        "res/sound/train/train_horn_1.wav", audio::SoundPlayback::ONCE, audio::SoundSpatiality::NONE);
    m_steam_loop_sound = audio::CreateSound(
        "res/sound/train/train_steam_loop.wav", audio::SoundPlayback::LOOPING, audio::SoundSpatiality::NONE);
    m_steam_loop_sound->SetPlaybackSpeed(tweak_values::steam_sound_idle_pitch);
    m_steam_loop_sound->Play();

    mono::ParticleSystem* particle_system = system_context->GetSystem<mono::ParticleSystem>();
    m_smoke_effect = std::make_unique<TrainSmokeEffect>(particle_system, m_entity_system, m_transform_system, m_entity_id);
    m_smoke_effect->Start();

    m_grind_effect = std::make_unique<WheelGrindEffect>(particle_system, m_entity_system);
    m_transform_system->ChildTransform(m_grind_effect->m_particle_entity, m_entity_id);

    m_aim_target = m_aim_direction = -math::PI_2();

    const TrainStateMachine::StateTable state_table = {
        TrainStateMachine::MakeState(TrainStates::DEFAULT, &TrainLogic::ToDefault, &TrainLogic::DefaultState, this),
        TrainStateMachine::MakeState(TrainStates::DEAD, &TrainLogic::ToDead, &TrainLogic::DeadState, &TrainLogic::ExitDead, this),
    };
    m_state.SetStateTableAndState(state_table, TrainStates::DEFAULT);

    m_hookshot = std::make_unique<Hookshot>(m_entity_id, m_entity_system, m_physics_system, m_sprite_system, m_transform_system, m_logic_system);

    // Make sure the player info is updated when constructed.
    UpdatePlayerInfo(0);
}

TrainLogic::~TrainLogic()
{
    Throw(0.0f);

    m_input_system->ReleaseContext(m_input_context);
    m_pickup_system->UnregisterPickupTarget(m_entity_id);
}

void TrainLogic::DrawDebugInfo(IDebugDrawer* debug_drawer) const
{
    const math::Vector world_position = m_transform_system->GetWorldPosition(m_entity_id);

    char buffer[512] = {};
    debug_drawer->DrawWorldText(buffer, world_position, mono::Color::OFF_WHITE);
}

const char* TrainLogic::GetDebugCategory() const
{
    return "train";
}

void TrainLogic::Update(const mono::UpdateContext& update_context)
{
    m_state.UpdateState(update_context);
    UpdatePlayerInfo(update_context.timestamp);
    UpdateSteamSound(update_context);
    UpdateTrainEffects(update_context);
}

void TrainLogic::UpdateController(const mono::UpdateContext& update_context)
{
    const uint32_t player_index = FindPlayerIndex(m_player_info);

    // Select most recent input if player zero, else just go with gamepad. 
    if(m_input_context->most_recent_input == mono::InputContextType::Controller || player_index > 0)
        m_gamepad_controller.Update(update_context);
    /*
    else
        m_keyboard_controller.Update(update_context);
    */
}

void TrainLogic::UpdatePlayerInfo(uint32_t timestamp)
{
    const math::Matrix& transform = m_transform_system->GetWorld(m_entity_id);
    const math::Vector last_position = m_player_info->position;
    const math::Vector current_position = math::GetPosition(transform);


    mono::IBody* body = m_physics_system->GetBody(m_entity_id);

    m_player_info->position = current_position;
    m_player_info->velocity = body->GetVelocity();
    m_player_info->direction = math::GetZRotation(transform);
    m_player_info->aim_direction = math::VectorFromAngle(m_aim_direction);
    m_player_info->aim_crosshair_screen_position = m_aim_screen_position;

    m_player_info->persistent_data.laser_sight = false;

    m_player_info->cooldown_id = 0;
    m_player_info->cooldown_fraction = 1.0f;

    const bool is_allocated = m_damage_system->IsAllocated(m_entity_id);
    if(is_allocated)
    {
        const DamageRecord* player_damage_record = m_damage_system->GetDamageRecord(m_entity_id);
        m_player_info->health_fraction = float(player_damage_record->health) / float(player_damage_record->full_health);
    }

    const PlayerLevelExperience& player_levels = GetPlayerLevelExperience(m_player_info->persistent_data.experience, m_config.player_levels);
    m_player_info->player_level = player_levels.level;

    m_player_info->stamina_fraction = 1.0f;
    m_player_info->player_experience_fraction =
        math::Scale01Clamped(float(m_player_info->persistent_data.experience), float(player_levels.current_level_experience), float(player_levels.next_level_experience));
    m_player_info->weapon_experience_fraction = 0.0f;

    m_player_info->active_weapon_modifiers.clear();
    m_player_info->last_used_input = m_input_context->most_recent_input;
}

void TrainLogic::UpdateSteamSound(const mono::UpdateContext& update_context)
{
    // Keeps chugging at an idle pitch even at a standstill, rather than starting/stopping
    // the loop outright - smoothed so the pitch eases between idle and full speed.
    const float speed = math::Length(m_player_info->velocity);
    const float speed_fraction = math::Scale01Clamped(speed, 0.0f, tweak_values::steam_sound_max_speed);
    const float pitch_range = tweak_values::steam_sound_max_pitch - tweak_values::steam_sound_idle_pitch;
    const float target_pitch = tweak_values::steam_sound_idle_pitch + (pitch_range * speed_fraction);

    math::simple_spring_damper_implicit(
        m_steam_pitch, m_steam_pitch_velocity, target_pitch, tweak_values::steam_sound_pitch_halflife, update_context.delta_s);

    m_steam_loop_sound->SetPlaybackSpeed(m_steam_pitch);
}

void TrainLogic::UpdateTrainEffects(const mono::UpdateContext& update_context)
{
    const float train_speed = math::Length(m_player_info->velocity);
    const float train_speed_fraction = math::Scale01Clamped(train_speed, 0.0f, 2.5f);
    const float emitter_speed = math::FractionToRange(train_speed_fraction, 1.0f, 5.0f);
    m_smoke_effect->UpdateEmitterSpeed(emitter_speed);

    const float path_curvature = m_path_follower_system->GetCurvature(m_entity_id);
    const float cornering_metric = train_speed * std::abs(path_curvature);

    if(cornering_metric >= tweak_values::grind_cornering_threshold)
    {
        // GetCurvature is signed relative to the path's own forward direction (increasing
        // distance), not the train's actual heading - running the same bend in reverse
        // turns the opposite way relative to the train, so flip it by the throttle sign.
        const float throttle = m_path_follower_system->GetThrottle(m_entity_id);
        const float curvature = (throttle >= 0.0f) ? path_curvature : -path_curvature;

        // World space is y-up, so positive curvature is a left (CCW) turn - spray sparks
        // out to the right; negative curvature is a right turn - spray out to the left.
        const float travel_direction = math::AngleFromVector(m_player_info->velocity);
        const float perpendicular_offset = (curvature >= 0.0f) ? -math::PI_4() : math::PI_4();
        const float direction = travel_direction + perpendicular_offset;
    
        const math::Vector& world_position = m_transform_system->GetWorldPosition(m_entity_id);
        m_grind_effect->EmitAtWithDirection(world_position, direction);
        m_grind_effect->Start();
    }
    else
    {
        m_grind_effect->Stop();
    }
}

void TrainLogic::UpdateAnimation(const mono::UpdateContext& update_context, float aim_direction, const math::Vector& world_position, const math::Vector& player_velocity)
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

    const math::Vector aim_target_vector = math::VectorFromAngle(m_aim_target);
    const math::Vector aim_direction_vector = math::VectorFromAngle(m_aim_direction);
    const float delta_angle_between = math::AngleBetweenPoints(aim_target_vector, aim_direction_vector);

    math::simple_spring_damper_implicit(
        m_aim_direction, m_aim_velocity, m_aim_direction - delta_angle_between, 0.1f, update_context.delta_s);
    m_aim_direction = math::NormalizeAngle(m_aim_direction);
}

void TrainLogic::ToDefault()
{ }

void TrainLogic::DefaultState(const mono::UpdateContext& update_context)
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

    UpdateAnimation(update_context, m_aim_direction, position, m_player_info->velocity);

    m_hookshot->Update(update_context);

    if(m_player_info->player_state == PlayerState::DEAD)
        m_state.TransitionTo(TrainStates::DEAD);
}

void TrainLogic::ToDead()
{
    if(m_death_anim_id != -1)
    {
        mono::ISprite* sprite = m_sprite_system->GetSprite(m_entity_id);
        sprite->SetAnimation(m_death_anim_id);
    }

    m_sprite_system->SetSpriteEnabled(m_entity_id, false);
    m_light_system->SetLightEnabled(m_entity_id, false);
    m_target_system->SetTargetEnabled(m_entity_id, false);

    if(HoldingPickup())
        Throw(0.0f);
}

void TrainLogic::DeadState(const mono::UpdateContext& update_context)
{
    UpdateController(update_context);

    if(m_player_info->player_state == PlayerState::ALIVE)
        m_state.TransitionTo(TrainStates::DEFAULT);
}

void TrainLogic::ExitDead()
{
    m_sprite_system->SetSpriteEnabled(m_entity_id, true);
    m_light_system->SetLightEnabled(m_entity_id, true);
    m_target_system->SetTargetEnabled(m_entity_id, true);
}

void TrainLogic::HandlePickup(PickupType type, int meta_data)
{
    m_player_info->persistent_data.experience =
        std::clamp(m_player_info->persistent_data.experience + 10, 0, m_config.max_experience);

    switch(type)
    {
    case PickupType::AMMO:
    {
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
        break;
    }
    case PickupType::COINS:
    {
        m_player_info->persistent_data.chips += meta_data;
        break;
    }
    case PickupType::EXPERIENCE:
    {
        break;
    }
    case PickupType::WEAPON_MODIFIER:
    {
        break;
    }
    };
}

void TrainLogic::TriggerHookshot()
{
    const math::Vector& position = m_transform_system->GetWorldPosition(m_entity_id);
    m_hookshot->TriggerHookshot(position, m_aim_direction);
}

void TrainLogic::ReleaseHookshot()
{
    m_hookshot->DetachHookshot();
}

void TrainLogic::Throw(float throw_force)
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
    m_drop_box_sound->Play();

    m_picked_up_id = mono::INVALID_ID;
}

void TrainLogic::ThrowAction()
{
    Throw(100.0f);
}

void TrainLogic::PickupDrop()
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
            m_pickup_box_sound->Play();

            // Must handle destroyed package while holding it and then player death.

            //const mono::ReleaseCallback release_callback = [this](uint32_t entity_id) {
            //};
            //const uint32_t m_package_release_callback = m_entity_system->AddReleaseCallback(m_picked_up_id, release_callback);
        }
        else if(interaction_type == InteractionType::WEAPON)
        {
        }
    };
    m_interaction_system->TryTriggerInteraction(m_entity_id, interaction_callback);
}

bool TrainLogic::HoldingPickup() const
{
    return (m_picked_up_id != mono::INVALID_ID);
}

void TrainLogic::SetThrottle(float throttle)
{
    m_path_follower_system->SetThrottle(m_entity_id, throttle);
}

void TrainLogic::Honk()
{
    m_horn_sound->Play();
    m_railway_system->ToggleSwitchAhead(m_entity_id);
}

void TrainLogic::RespawnPlayer()
{
    m_event_handler->DispatchEvent(game::RespawnPlayerEvent(m_entity_id));
}

void TrainLogic::TogglePauseGame()
{
    m_pause = !m_pause;
    m_event_handler->DispatchEvent(event::PauseEvent(m_pause));
}

