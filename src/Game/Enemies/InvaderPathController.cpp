
#include "InvaderPathController.h"

#include "AIKnowledge.h"
#include "Factories.h"
#include "Behaviour/PathBehaviour.h"
#include "Weapons/IWeaponSystem.h"
#include "Weapons/IWeaponFactory.h"

#include "Paths/IPath.h"
#include "Math/MathFunctions.h"

using namespace game;

InvaderPathController::InvaderPathController(mono::IPathPtr& path, mono::EventHandler& event_handler)
    : m_path(std::move(path))
    , m_event_handler(event_handler)
    , m_fire_count(0)
    , m_fire_cooldown(0)
{ }

InvaderPathController::~InvaderPathController()
{ }

void InvaderPathController::Initialize(Enemy* enemy)
{
    m_enemy = enemy;
    m_path_behaviour = std::make_unique<PathBehaviour>(enemy, m_path.get(), m_event_handler);
    m_weapon = weapon_factory->CreateWeapon(WeaponType::GENERIC, WeaponFaction::ENEMY);
}

void InvaderPathController::doUpdate(unsigned int delta)
{
    m_path_behaviour->Run(delta);

    if(m_fire_cooldown > 0)
    {
        m_fire_cooldown -= delta;
        return;
    }

    if(!g_player_one.is_active)
        return;

    const math::Vector& enemy_position = m_enemy->Position();
    const bool is_visible = math::PointInsideQuad(enemy_position, g_camera_viewport);
    if(!is_visible)
        return;

    const float distance = math::Length(g_player_one.position - enemy_position);
    if(distance < 7.0f)
    {
        const float angle = math::AngleBetweenPoints(g_player_one.position, enemy_position) + math::PI_2();
        m_fire_count += (m_weapon->Fire(enemy_position, angle) == WeaponFireResult::FIRE);
    }

    if(m_fire_count == 5)
    {
        m_fire_cooldown = 2000;
        m_fire_count = 0;
    }
}
