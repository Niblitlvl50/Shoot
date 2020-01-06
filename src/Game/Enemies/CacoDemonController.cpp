
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

#include "SystemContext.h"
#include "TransformSystem/TransformSystem.h"

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

CacodemonController::CacodemonController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler& event_handler)
    : m_entity_id(entity_id)
{
    m_weapon = g_weapon_factory->CreateWeapon(WeaponType::CACOPLASMA, WeaponFaction::ENEMY);

    mono::TransformSystem* transform_system = system_context->GetSystem<mono::TransformSystem>();
    m_transform = &transform_system->GetTransform(entity_id);

    mono::PhysicsSystem* physics_system = system_context->GetSystem<mono::PhysicsSystem>();

    m_entity_body = physics_system->GetBody(entity_id);
    m_tracking_behaviour = std::make_unique<TrackingBehaviour>(m_entity_body, physics_system);
    m_tracking_behaviour->SetTrackingSpeed(1.0f);

    mono::SpriteSystem* sprite_system = system_context->GetSystem<mono::SpriteSystem>();
    sprite_system->GetSprite(entity_id)->SetAnimation(Animation::IDLE);
}

CacodemonController::~CacodemonController()
{ }

void CacodemonController::Update(uint32_t delta_ms)
{
    if(!g_player_one.is_active)
        return;

    const TrackingResult result = m_tracking_behaviour->Run(delta_ms);

    const math::Vector& position = math::GetPosition(*m_transform);
    const float rotation = math::GetZRotation(*m_transform);
    
    if(result == TrackingResult::TRACKING)
    {
        const float angle = math::AngleBetweenPoints(g_player_one.position, position) + math::PI_2();
        m_entity_body->SetAngle(angle);
    }

    const bool is_visible = math::PointInsideQuad(position, g_camera_viewport);
    if(is_visible)
    {
        const WeaponFireResult fire_result = m_weapon->Fire(position, rotation);
        if(fire_result == WeaponFireResult::OUT_OF_AMMO)
            m_weapon->Reload();
    }
}
