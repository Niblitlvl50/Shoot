
#include "InvaderPathController.h"

#include "Entity/TargetSystem.h"
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

InvaderPathController::InvaderPathController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler* event_handler)
    : m_fire_count(0)
    , m_fire_cooldown_s(0.0f)
{
}

InvaderPathController::InvaderPathController(uint32_t entity_id, uint32_t path_entity_id, mono::SystemContext* system_context, mono::EventHandler* event_handler)
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

    m_target_system = system_context->GetSystem<TargetSystem>();
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

    const math::Vector& world_position = math::GetPosition(*m_transform);
    m_aquired_target = m_target_system->AquireTarget(TargetFaction::Player, world_position, tweak_values::attack_distance);
    if(!m_aquired_target->IsValid())
        return;

//    const bool is_visible = math::PointInsideQuad(world_position, player_info->viewport);
//    if(!is_visible)
//        return;

    const bool did_fire = (m_weapon->Fire(world_position, m_aquired_target->Position(), update_context.timestamp) == WeaponState::FIRE);
    m_fire_count += did_fire ? 1 : 0;
    
    if(m_fire_count == 5)
    {
        m_fire_cooldown_s = 2.0f;
        m_fire_count = 0;
    }
}
