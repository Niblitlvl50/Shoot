
#include "PlayerLogic.h"
#include "AIKnowledge.h"

#include "SystemContext.h"
#include "TransformSystem/TransformSystem.h"
#include "Physics/PhysicsSystem.h"
#include "Rendering/Sprite/Sprite.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Particle/ParticleSystem.h"

#include "Factories.h"
#include "Weapons/IWeapon.h"
#include "Weapons/IWeaponFactory.h"

#include "EntitySystem/IEntityManager.h"
#include "Math/MathFunctions.h"

#include "Effects/TrailEffect.h"
#include "Effects/BlinkEffect.h"
#include "Pickups/PickupSystem.h"

#include "Component.h"

#include <cmath>

namespace tweak_values
{
    constexpr uint32_t blink_duration_ms = 200;
    constexpr float blink_distance = 2.0f;
}

using namespace game;

PlayerLogic::PlayerLogic(
    uint32_t entity_id,
    PlayerInfo* player_info,
    mono::EventHandler& event_handler,
    const System::ControllerState& controller,
    mono::SystemContext* system_context)
    : m_entity_id(entity_id)
    , m_player_info(player_info)
    , m_gamepad_controller(this, event_handler, controller)
    , m_fire(false)
    , m_secondary_fire(false)
    , m_total_ammo_left(500)
    , m_aim_direction(0.0f)
{
    m_transform_system = system_context->GetSystem<mono::TransformSystem>();
    m_physics_system = system_context->GetSystem<mono::PhysicsSystem>();
    m_sprite_system = system_context->GetSystem<mono::SpriteSystem>();
    m_pickup_system = system_context->GetSystem<PickupSystem>();

    const PickupCallback pickup_callback = [this](shared::PickupType type, int amount) {
        switch(type)
        {
        case shared::PickupType::AMMO:
            m_total_ammo_left += amount;
            break;
        case shared::PickupType::HEALTH:
            std::printf("Got Health! (%d)\n", amount);
            break;
        case shared::PickupType::SCORE:
            m_player_info->score += amount;
            break;
        };
    };

    m_pickup_system->RegisterPickupTarget(m_entity_id, pickup_callback);

    mono::ParticleSystem* particle_system = system_context->GetSystem<mono::ParticleSystem>();
    //m_trail_effect = std::make_unique<TrailEffect>(m_transform_system, particle_system, entity_id);
    m_blink_effect = std::make_unique<BlinkEffect>(particle_system);
    m_blink_sound = mono::AudioFactory::CreateSound("res/sound/punch.wav", mono::SoundPlayback::ONCE, mono::SoundPosition::GLOBAL);

    // Make sure we have a weapon
    SelectWeapon(WeaponType::STANDARD);
    SelectSecondaryWeapon(WeaponType::ROCKET_LAUNCHER);
    SetRotation(0.0f);

    const PlayerStateMachine::StateTable state_table = {
        PlayerStateMachine::MakeState(PlayerStates::DEFAULT, &PlayerLogic::ToDefault, &PlayerLogic::DefaultState, this),
        PlayerStateMachine::MakeState(PlayerStates::BLINK, &PlayerLogic::ToBlink, &PlayerLogic::BlinkState, this),
    };

    m_state.SetStateTableAndState(state_table, PlayerStates::DEFAULT);
}

PlayerLogic::~PlayerLogic()
{
    m_pickup_system->UnregisterPickupTarget(m_entity_id);
}

void PlayerLogic::Update(const mono::UpdateContext& update_context)
{
    const math::Matrix& transform = m_transform_system->GetWorld(m_entity_id);
    const math::Vector& position = math::GetPosition(transform);
    const float direction = math::GetZRotation(transform);

    if(m_fire)
        m_player_info->weapon_state = m_weapon->Fire(position, m_aim_direction, update_context.timestamp);

    if(m_secondary_fire)
    {
        m_secondary_weapon->Fire(position, m_aim_direction, update_context.timestamp);
        m_secondary_fire = false;
    }

    mono::IBody* body = m_physics_system->GetBody(m_entity_id);

    m_player_info->position = position;
    m_player_info->velocity = body->GetVelocity();
    m_player_info->direction = direction;

    m_player_info->magazine_left = m_weapon->AmmunitionLeft();
    m_player_info->magazine_capacity = m_weapon->MagazineSize();
    m_player_info->ammunition_left = m_total_ammo_left;
    m_player_info->weapon_type = m_weapon_type;
    m_player_info->weapon_state = m_weapon->GetState();

    m_state.UpdateState(update_context);
}

void PlayerLogic::ToDefault()
{
    m_sprite_system->SetSpriteEnabled(m_entity_id, true);
}

void PlayerLogic::DefaultState(const mono::UpdateContext& update_context)
{
    m_gamepad_controller.Update(update_context);
}

void PlayerLogic::ToBlink()
{
    m_sprite_system->SetSpriteEnabled(m_entity_id, false);

    const math::Matrix& transform = m_transform_system->GetWorld(m_entity_id);
    const math::Vector& position = math::GetPosition(transform);
    m_blink_effect->EmitBlinkAwayAt(position);

    m_blink_sound->Position(position.x, position.y);
    m_blink_sound->Play();

    m_blink_counter = 0;
}

void PlayerLogic::BlinkState(const mono::UpdateContext& update_context)
{
    m_blink_counter += update_context.delta_ms;

    if(m_blink_counter >= tweak_values::blink_duration_ms)
    {
        mono::IBody* body = m_physics_system->GetBody(m_entity_id);
        math::Vector new_position = body->GetPosition();

        switch(m_blink_direction)
        {
        case BlinkDirection::LEFT:
            new_position.x -= tweak_values::blink_distance;
            break;
        case BlinkDirection::RIGHT:
            new_position.x += tweak_values::blink_distance;
            break;
        case BlinkDirection::UP:
            new_position.y += tweak_values::blink_distance;
            break;
        case BlinkDirection::DOWN:
            new_position.y -= tweak_values::blink_distance;
            break;
        }

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
    m_secondary_weapon->Reload(timestamp);
}

void PlayerLogic::SecondaryFire()
{
    m_secondary_fire = true;
}

void PlayerLogic::SelectWeapon(WeaponType weapon)
{
    m_weapon = g_weapon_factory->CreateWeapon(weapon, WeaponFaction::PLAYER, m_entity_id);
    m_weapon_type = weapon;
}

void PlayerLogic::SelectSecondaryWeapon(WeaponType weapon)
{
    m_secondary_weapon = g_weapon_factory->CreateWeapon(weapon, WeaponFaction::PLAYER, m_entity_id);
}

void PlayerLogic::ApplyImpulse(const math::Vector& force)
{
    mono::IBody* body = m_physics_system->GetBody(m_entity_id);
    body->ApplyLocalImpulse(force, math::ZeroVec);
}

void PlayerLogic::ApplyForce(const math::Vector& force)
{
    mono::IBody* body = m_physics_system->GetBody(m_entity_id);
    body->ApplyLocalForce(force, math::ZeroVec);
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
}

void PlayerLogic::SetRotation(float rotation)
{
    m_aim_direction = rotation;

    //mono::Sprite* sprite = m_sprite_system->GetSprite(m_weapon_entity_id);
    //const auto direction = (rotation < math::PI()) ? mono::VerticalDirection::UP : mono::VerticalDirection::DOWN;
    //sprite->SetVerticalDirection(direction);

    //math::Matrix weapon_transform = math::CreateMatrixFromZRotation(rotation - math::PI_2());
    //math::Translate(weapon_transform, math::Vector(0.25f, 1.0f));

    //m_transform_system->SetTransform(m_weapon_entity_id, weapon_transform);
}

void PlayerLogic::SetAnimation(PlayerAnimation animation)
{
    //const mono::VerticalDirection vertical = 
    //    delta_position.y < 0.0f ? mono::VerticalDirection::UP : mono::VerticalDirection::DOWN;

    //mono::Sprite* sprite = m_sprite_system->GetSprite(m_entity_id);

    switch(animation)
    {
        case PlayerAnimation::IDLE:
            //sprite->SetAnimation(STANDING);
            break;
        case PlayerAnimation::DUCK:
            //sprite->SetAnimation(DUCKING);
            break;
        case PlayerAnimation::WALK_LEFT:
        case PlayerAnimation::WALK_RIGHT:
        {
            //sprite->SetAnimation(WALKING);
            //const auto direction = 
            //    (animation == PlayerAnimation::WALK_LEFT) ? mono::HorizontalDirection::LEFT : mono::HorizontalDirection::RIGHT;
            //sprite->SetHorizontalDirection(direction);
            break;
        }
        case PlayerAnimation::WALK_UP:
            //sprite->SetAnimation(CLIMBING);
            break;
    };

    //sprite->SetHorizontalDirection(horizontal);
    //sprite->SetVerticalDirection(vertical);
}

void PlayerLogic::Blink(BlinkDirection direction)
{
    m_blink_direction = direction;
    m_state.TransitionTo(PlayerStates::BLINK);
}
