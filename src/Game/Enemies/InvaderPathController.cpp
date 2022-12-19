
#include "InvaderPathController.h"

#include "Player/PlayerInfo.h"
#include "Behaviour/PathBehaviour.h"
#include "Weapons/IWeapon.h"
#include "Weapons/WeaponSystem.h"

#include "Paths/IPath.h"
#include "Math/MathFunctions.h"

#include "SystemContext.h"
#include "TransformSystem/TransformSystem.h"
#include "Physics/PhysicsSystem.h"
#include "Paths/PathSystem.h"
#include "Paths/PathFactory.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Sprite/Sprite.h"
#include "Rendering/Sprite/SpriteProperties.h"

namespace tweak_values
{
    constexpr float attack_distance = 5.0f;
    constexpr float path_speed = 1.0f;
}

using namespace game;

InvaderPathController::InvaderPathController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler& event_handler)
    : m_fire_count(0)
    , m_fire_cooldown_s(0.0f)
    //, m_path(std::move(path))
{
}

InvaderPathController::InvaderPathController(uint32_t entity_id, uint32_t path_entity_id, mono::SystemContext* system_context, mono::EventHandler& event_handler)
    : m_fire_count(0)
    , m_fire_cooldown_s(0.0f)
{
    mono::TransformSystem* transform_system = system_context->GetSystem<mono::TransformSystem>();
    m_transform = &transform_system->GetTransform(entity_id);

    mono::SpriteSystem* sprite_system = system_context->GetSystem<mono::SpriteSystem>();
    m_sprite = sprite_system->GetSprite(entity_id);

    mono::PhysicsSystem* physics_system = system_context->GetSystem<mono::PhysicsSystem>();
    m_body = physics_system->GetBody(entity_id);

    mono::PathSystem* path_system = system_context->GetSystem<mono::PathSystem>();
    const mono::PathComponent* path_component = path_system->GetPath(path_entity_id);
    const math::Matrix& path_entity_transform = transform_system->GetWorld(path_entity_id);

    m_path = mono::CreatePath(path_component->points, path_entity_transform);
    m_path_behaviour = std::make_unique<PathBehaviour>(m_body, m_path.get(), physics_system);
    m_path_behaviour->SetTrackingSpeed(tweak_values::path_speed);

    game::WeaponSystem* weapon_system = system_context->GetSystem<game::WeaponSystem>();
    m_weapon = weapon_system->CreatePrimaryWeapon(entity_id, WeaponFaction::ENEMY);
}

InvaderPathController::~InvaderPathController()
{ }

void InvaderPathController::Update(const mono::UpdateContext& update_context)
{
    m_path_behaviour->Run(update_context);

    const math::Vector velocity = m_body->GetVelocity();
    const bool is_going_left = (velocity.x < 0.0f);
    is_going_left ?
        m_sprite->SetProperty(mono::SpriteProperty::FLIP_HORIZONTAL) : m_sprite->ClearProperty(mono::SpriteProperty::FLIP_HORIZONTAL);

    if(m_fire_cooldown_s > 0.0f)
    {
        m_fire_cooldown_s -= update_context.delta_s;
        return;
    }

    const math::Vector& enemy_position = math::GetPosition(*m_transform);
    const game::PlayerInfo* player_info = GetClosestActivePlayer(enemy_position);
    if(!player_info)
        return;

    const bool is_visible = math::PointInsideQuad(enemy_position, player_info->viewport);
    if(!is_visible)
        return;

    const float distance = math::Length(player_info->position - enemy_position);
    if(distance < tweak_values::attack_distance)
    {
        const float angle = math::AngleBetweenPoints(player_info->position, enemy_position);
        m_fire_count += (m_weapon->Fire(enemy_position, angle, update_context.timestamp) == WeaponState::FIRE);
    }

    if(m_fire_count == 5)
    {
        m_fire_cooldown_s = 2.0f;
        m_fire_count = 0;
    }
}
