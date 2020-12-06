
#include "CacoDemonController.h"

#include "AIKnowledge.h"
#include "Behaviour/TrackingBehaviour.h"
#include "Factories.h"
#include "Weapons/IWeaponSystem.h"
#include "Weapons/IWeaponFactory.h"

#include "Math/MathFunctions.h"
#include "Physics/PhysicsSystem.h"
#include "Rendering/Sprite/ISprite.h"
#include "Rendering/Sprite/Sprite.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Util/Random.h"

#include "SystemContext.h"
#include "TransformSystem/TransformSystem.h"

#include <cmath>

namespace tweak_values
{
    constexpr float radians_per_second = math::ToRadians(30.0f);
    constexpr float face_angle = math::ToRadians(5.0f);
}

using namespace game;

CacodemonController::CacodemonController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler& event_handler)
    : m_entity_id(entity_id)
{
    m_weapon = g_weapon_factory->CreateWeapon(WeaponType::CACOPLASMA, WeaponFaction::ENEMY, entity_id);

    mono::TransformSystem* transform_system = system_context->GetSystem<mono::TransformSystem>();
    m_transform = &transform_system->GetTransform(entity_id);

    mono::PhysicsSystem* physics_system = system_context->GetSystem<mono::PhysicsSystem>();
    m_entity_body = physics_system->GetBody(entity_id);

    mono::SpriteSystem* sprite_system = system_context->GetSystem<mono::SpriteSystem>();
    sprite_system->SetSpriteLayer(entity_id, -1);

    m_entity_sprite = sprite_system->GetSprite(entity_id);

    m_idle_animation = m_entity_sprite->GetAnimationIdFromName("idle");
    m_attack_animation = m_entity_sprite->GetAnimationIdFromName("attack");
    //m_entity_sprite->GetAnimationIdFromName("death");

    const CacoStateMachine::StateTable state_table = {
        CacoStateMachine::MakeState(CacoStates::IDLE, &CacodemonController::OnIdle, &CacodemonController::Idle, this),
        CacoStateMachine::MakeState(CacoStates::ATTACK, &CacodemonController::OnAttack, &CacodemonController::Attack, this),
    };

    m_states.SetStateTableAndState(state_table, CacoStates::IDLE);
}

CacodemonController::~CacodemonController()
{ }

void CacodemonController::Update(const mono::UpdateContext& update_context)
{
    m_states.UpdateState(update_context);
}

void CacodemonController::OnIdle()
{
    m_entity_sprite->SetAnimation(m_idle_animation);
}

void CacodemonController::Idle(const mono::UpdateContext& update_context)
{
    const math::Vector position = math::GetPosition(*m_transform);
    const bool is_visible = math::PointInsideQuad(position, g_camera_viewport);
    if(!is_visible || !g_player_one.is_active)
        return;

    const float new_angle = math::AngleBetweenPoints(g_player_one.position, position);
    const float current_rotation = math::GetZRotation(*m_transform);

    const float angle_diff = current_rotation - new_angle - math::PI_2();
    const float angle_diff_abs = std::fabsf(math::NormalizeAngle(angle_diff) - math::PI() * 2.0f);

    if(angle_diff_abs <= tweak_values::face_angle)
    {
        m_states.TransitionTo(CacoStates::ATTACK);
    }
    else
    {
        const float direction = angle_diff > 0.0f ? -1.0f : 1.0f;
        const float add = tweak_values::radians_per_second * float(update_context.delta_ms) / 1000.0f * direction;
        m_entity_body->SetAngle(current_rotation + add);
    }
}

void CacodemonController::OnAttack()
{
    m_entity_sprite->SetAnimation(m_attack_animation);
}

void CacodemonController::Attack(const mono::UpdateContext& update_context)
{
    const math::Vector position = math::GetPosition(*m_transform);
    const float rotation = math::GetZRotation(*m_transform) + math::PI();
    const math::Vector offset_vector = math::VectorFromAngle(rotation) * 0.5f;

    const WeaponState fire_result = m_weapon->Fire(position + offset_vector, rotation, update_context.timestamp);
    if(fire_result == WeaponState::OUT_OF_AMMO)
    {
        m_weapon->Reload(update_context.timestamp);
        m_states.TransitionTo(CacoStates::IDLE);
    }
}
