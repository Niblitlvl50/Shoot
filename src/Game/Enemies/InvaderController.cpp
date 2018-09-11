
#include "InvaderController.h"

#include "AIKnowledge.h"
#include "Behaviour/TrackingBehaviour.h"
#include "Factories.h"
#include "Weapons/IWeaponSystem.h"
#include "Weapons/IWeaponFactory.h"

#include "Rendering/Color.h"
#include "Rendering/Sprite/ISprite.h"
#include "Math/MathFunctions.h"

using namespace game;

InvaderController::InvaderController(mono::EventHandler& event_handler)
    : m_event_handler(event_handler)
{
    using namespace std::placeholders;

    const std::unordered_map<InvaderStates, InvaderStateMachine::State>& state_table = {
        { InvaderStates::IDLE,      { std::bind(&InvaderController::ToIdle,      this), std::bind(&InvaderController::Idle,      this, _1) } },
        { InvaderStates::TRACKING,  { std::bind(&InvaderController::ToTracking,  this), std::bind(&InvaderController::Tracking,  this, _1) } },
        { InvaderStates::ATTACKING, { std::bind(&InvaderController::ToAttacking, this), std::bind(&InvaderController::Attacking, this, _1) } }
    };

    m_states.SetStateTable(state_table);
}

InvaderController::~InvaderController()
{ }

void InvaderController::Initialize(Enemy* enemy)
{
    m_enemy = enemy;
    m_weapon = weapon_factory->CreateWeapon(WeaponType::GENERIC, WeaponFaction::ENEMY);
    m_tracking_behaviour = std::make_unique<TrackingBehaviour>(m_enemy, m_event_handler);
    m_states.TransitionTo(InvaderStates::IDLE);
}

void InvaderController::doUpdate(unsigned int delta)
{
    m_states.UpdateState(delta);
}

void InvaderController::ToIdle()
{
    constexpr mono::Color::RGBA color(0.2, 0.2, 0.2);
    m_enemy->m_sprite->SetShade(color);

    m_idle_timer = 0;
}

void InvaderController::ToTracking()
{
    constexpr mono::Color::RGBA color(1.0f, 0.0f, 0.0f);
    m_enemy->m_sprite->SetShade(color);
}

void InvaderController::ToAttacking()
{

}

void InvaderController::Idle(unsigned int delta)
{
    m_idle_timer += delta;
    if(m_idle_timer > 2000 && g_player_one.is_active)
        m_states.TransitionTo(InvaderStates::TRACKING);
}

void InvaderController::Tracking(unsigned int delta)
{
    const float distance_to_player = math::Length(g_player_one.position - m_enemy->Position());
    if(distance_to_player < 5.0f)
    {
        m_states.TransitionTo(InvaderStates::ATTACKING);
        return;
    }

    const TrackingResult result = m_tracking_behaviour->Run(delta);
    if(result == TrackingResult::NO_PATH || result == TrackingResult::AT_TARGET)
        m_states.TransitionTo(InvaderStates::IDLE);
}

void InvaderController::Attacking(unsigned int delta)
{
    const float distance_to_player = math::Length(g_player_one.position - m_enemy->Position());
    if(distance_to_player > 10.0f || !g_player_one.is_active)
    {
        m_states.TransitionTo(InvaderStates::TRACKING);
        return;
    }

    const math::Vector& enemy_position = m_enemy->Position();
    const float angle = math::AngleBetweenPoints(g_player_one.position, enemy_position) + math::PI_2();
    m_weapon->Fire(enemy_position, angle);
}
