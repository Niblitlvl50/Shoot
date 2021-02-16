
#include "InvaderPathController.h"

#include "Player/PlayerInfo.h"
#include "Factories.h"
#include "Behaviour/PathBehaviour.h"
#include "Weapons/IWeapon.h"

#include "Paths/IPath.h"
#include "Math/MathFunctions.h"

#include "SystemContext.h"
#include "TransformSystem/TransformSystem.h"
#include "Physics/PhysicsSystem.h"

using namespace game;

InvaderPathController::InvaderPathController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler& event_handler)
    : m_fire_count(0)
    , m_fire_cooldown(0)
    //, m_path(std::move(path))
{
    mono::TransformSystem* transform_system = system_context->GetSystem<mono::TransformSystem>();
    m_transform = &transform_system->GetTransform(entity_id);

    //m_path_behaviour = std::make_unique<PathBehaviour>(enemy, m_path.get(), m_event_handler);
    m_weapon = g_weapon_factory->CreateWeapon(WeaponType::GENERIC, WeaponFaction::ENEMY, entity_id);
}

InvaderPathController::InvaderPathController(uint32_t entity_id, mono::IPathPtr path, mono::SystemContext* system_context, mono::EventHandler& event_handler)
    : m_fire_count(0)
    , m_fire_cooldown(0)
    , m_path(std::move(path))
{
    mono::TransformSystem* transform_system = system_context->GetSystem<mono::TransformSystem>();
    m_transform = &transform_system->GetTransform(entity_id);

    mono::PhysicsSystem* physics_system = system_context->GetSystem<mono::PhysicsSystem>();
    mono::IBody* entity_body = physics_system->GetBody(entity_id);

    m_path_behaviour = std::make_unique<PathBehaviour>(entity_body, m_path.get(), physics_system, event_handler);
    m_weapon = g_weapon_factory->CreateWeapon(WeaponType::GENERIC, WeaponFaction::ENEMY, entity_id);
}

InvaderPathController::~InvaderPathController()
{ }

void InvaderPathController::Update(const mono::UpdateContext& update_context)
{
    m_path_behaviour->Run(update_context.delta_ms);

    if(m_fire_cooldown > 0)
    {
        m_fire_cooldown -= update_context.delta_ms;
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
    if(distance < 7.0f)
    {
        const float angle = math::AngleBetweenPoints(player_info->position, enemy_position) + math::PI_2();
        m_fire_count += (m_weapon->Fire(enemy_position, angle, update_context.timestamp) == WeaponState::FIRE);
    }

    if(m_fire_count == 5)
    {
        m_fire_cooldown = 2000;
        m_fire_count = 0;
    }
}
