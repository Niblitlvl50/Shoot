
#include "CacoDemonController.h"

#include "AIKnowledge.h"
#include "Behaviour/TrackingBehaviour.h"
#include "Factories.h"
#include "Weapons/IWeaponSystem.h"
#include "Weapons/IWeaponFactory.h"

#include "Math/MathFunctions.h"
#include "Rendering/Sprite/ISprite.h"

#include <cmath>

using namespace game;

namespace
{
    enum Animation
    {
        IDLE,
        ATTACK,
        HURT,
        DEATH
    };
}

CacoDemonController::CacoDemonController(mono::EventHandler& event_handler)
    : m_event_handler(event_handler)
{
    m_weapon = weapon_factory->CreateWeapon(WeaponType::CACOPLASMA, WeaponFaction::ENEMY);
}

CacoDemonController::~CacoDemonController()
{ }

void CacoDemonController::Initialize(Enemy* enemy)
{
    m_enemy = enemy;
    m_enemy->m_sprite->SetAnimation(Animation::ATTACK);
    m_tracking_behaviour = std::make_unique<TrackingBehaviour>(m_enemy, m_event_handler);
    m_tracking_behaviour->SetTrackingSpeed(1.0f);
}

void CacoDemonController::doUpdate(unsigned int delta)
{
    if(!g_player_one.is_active)
        return;

    const TrackingResult result = m_tracking_behaviour->Run(delta);

    if(result == TrackingResult::TRACKING)
    {
        const float angle = math::AngleBetweenPoints(g_player_one.position, m_enemy->Position()) + math::PI_2();
        m_enemy->SetRotation(angle);
    }

    
    const bool is_visible = math::PointInsideQuad(m_enemy->Position(), g_camera_viewport);

    //const float distance = math::Length(g_player_one.position - m_enemy->Position());
    //if(distance < 10.0f)
    if(is_visible)
    {
        const WeaponFireResult fire_result = m_weapon->Fire(m_enemy->Position(), m_enemy->Rotation());
        if(fire_result == WeaponFireResult::OUT_OF_AMMO)
            m_weapon->Reload();
    }
}
