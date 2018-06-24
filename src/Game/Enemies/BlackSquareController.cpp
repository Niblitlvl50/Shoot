
#include "BlackSquareController.h"
#include "EntityProperties.h"
#include "AIKnowledge.h"
#include "Events/SpawnConstraintEvent.h"
#include "Explosion.h"
#include "RenderLayers.h"
#include "CollisionConfiguration.h"

#include "Events/DamageEvent.h"
#include "Events/SpawnEntityEvent.h"
#include "Events/RemoveEntityEvent.h"
#include "Events/ShockwaveEvent.h"


#include "Physics/CMFactory.h"
#include "Physics/IBody.h"
#include "Physics/IConstraint.h"
#include "EventHandler/EventHandler.h"
#include "Rendering/Sprite/ISprite.h"
#include "Rendering/Color.h"
#include "Math/MathFunctions.h"

using namespace game;


BlackSquareController::BlackSquareController(float trigger_distance, mono::EventHandler& event_handler)
    : m_triggerDistance(trigger_distance),
      m_eventHandler(event_handler),
      m_awakeStateTimer(0)
{
    using namespace std::placeholders;

    const std::unordered_map<States, MyStateMachine::State>& state_table = {
        { States::SLEEPING, { std::bind(&BlackSquareController::ToSleep, this), std::bind(&BlackSquareController::SleepState, this, _1) } },
        { States::AWAKE,    { std::bind(&BlackSquareController::ToAwake, this), std::bind(&BlackSquareController::AwakeState, this, _1) } },
        { States::HUNT,     { std::bind(&BlackSquareController::ToHunt,  this), std::bind(&BlackSquareController::HuntState, this, _1)  } }
    };

    m_states.SetStateTable(state_table);    
}

BlackSquareController::~BlackSquareController()
{
    m_eventHandler.DispatchEvent(DespawnConstraintEvent(m_spring));
}

void BlackSquareController::Initialize(Enemy* enemy)
{
    m_enemy = enemy;
    m_enemy->SetProperty(EntityProperties::DAMAGABLE);

    m_controlBody = mono::PhysicsFactory::CreateKinematicBody();
    m_controlBody->SetPosition(m_enemy->Position());

    mono::IBodyPtr enemy_body = m_enemy->GetPhysics().body;
    enemy_body->SetCollisionHandler(this);

    m_spring = mono::PhysicsFactory::CreateSpring(m_controlBody, enemy_body, 0.0f, 50.0f, 0.5f);
    m_eventHandler.DispatchEvent(SpawnConstraintEvent(m_spring));

    m_states.TransitionTo(States::SLEEPING);
}

void BlackSquareController::doUpdate(unsigned int delta)
{
    m_states.UpdateState(delta);
}

void BlackSquareController::OnCollideWith(const mono::IBodyPtr& body, unsigned int category)
{
    if(m_states.ActiveState() == States::SLEEPING)
        m_states.TransitionTo(States::AWAKE);

    if(category != game::CollisionCategory::PLAYER)
        return;

    const float direction = math::AngleBetweenPoints(m_enemy->Position(), body->GetPosition());

    game::ExplosionConfiguration explosion_config;
    explosion_config.position = m_enemy->Position();
    explosion_config.scale = 2.0f;
    explosion_config.rotation = 0.0f;
    explosion_config.sprite_file = "res/sprites/explosion.sprite";
    
    const game::SpawnEntityEvent event(
        std::make_shared<game::Explosion>(explosion_config, m_eventHandler), game::FOREGROUND);
    
    m_eventHandler.DispatchEvent(event);
    m_eventHandler.DispatchEvent(game::DamageEvent(body, 45, direction));
    m_eventHandler.DispatchEvent(game::ShockwaveEvent(explosion_config.position, 100));
    m_eventHandler.DispatchEvent(game::RemoveEntityEvent(m_enemy->Id()));
}

void BlackSquareController::ToSleep()
{
    m_enemy->m_sprite->SetShade(mono::Color::RGBA(0.0f, 0.0f, 0.0f, 1.0f));
    m_enemy->GetPhysics().body->ResetForces();
    m_controlBody->SetPosition(m_enemy->Position());
}

void BlackSquareController::ToAwake()
{
    m_enemy->m_sprite->SetShade(mono::Color::RGBA(1.0f, 1.0f, 1.0f, 1.0f));
    m_awakeStateTimer = 0;
}

void BlackSquareController::ToHunt()
{
    m_enemy->m_sprite->SetShade(mono::Color::RGBA(1.0f, 0.0f, 0.0f, 1.0f));
}

void BlackSquareController::SleepState(unsigned int delta)
{
    if(!g_player_one.is_active)
        return;

    m_controlBody->SetPosition(m_enemy->Position());
        
    const float distance = math::Length(g_player_one.position - m_enemy->Position());
    if(distance < m_triggerDistance)
        m_states.TransitionTo(States::AWAKE);
}

void BlackSquareController::AwakeState(unsigned int delta)
{
    m_awakeStateTimer += delta;
    if(m_awakeStateTimer > 300)
        m_states.TransitionTo(States::HUNT);
}

void BlackSquareController::HuntState(unsigned int delta)
{
    const float angle = math::AngleBetweenPoints(g_player_one.position, m_enemy->Position()) - math::PI_2();

    m_controlBody->SetPosition(g_player_one.position);
    m_enemy->SetRotation(angle);

    const float distance = math::Length(g_player_one.position - m_enemy->Position());
    if(distance > (m_triggerDistance * 2.0f) || !g_player_one.is_active)
        m_states.TransitionTo(States::SLEEPING);
}
