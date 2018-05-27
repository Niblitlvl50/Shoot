
#include "Shuttle.h"
#include "Physics/IBody.h"
#include "Physics/IShape.h"
#include "Physics/CMFactory.h"
#include "Random.h"

#include "Entity/EntityBase.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Sprite/ISprite.h"
#include "Rendering/Sprite/SpriteFactory.h"
#include "Rendering/Texture/TextureFactory.h"
#include "Rendering/Color.h"

#include "Particle/ParticlePool.h"
#include "Particle/ParticleDrawer.h"
#include "Particle/ParticleSystemDefaults.h"

#include "Math/MathFunctions.h"
#include "Math/Matrix.h"

#include "AIKnowledge.h"
#include "EntityProperties.h"
#include "CollisionConfiguration.h"

#include "Factories.h"
#include "Weapons/IWeaponSystem.h"
#include "Weapons/IWeaponFactory.h"

#include "Effects/TrailEffect.h"

#include <cmath>


using namespace game;

namespace constants
{    
    enum
    {
        IDLE = 0,
        THRUSTING
    };
}

class SpriteEntity : public mono::EntityBase
{
public:

    SpriteEntity(const char* sprite_file)
    {
        m_sprite = mono::CreateSprite(sprite_file);
    }

    virtual void Draw(mono::IRenderer& renderer) const
    {
        if(m_enabled)
            renderer.DrawSprite(*m_sprite);
    }

    virtual void Update(unsigned int delta)
    {
        m_sprite->doUpdate(delta);
    }

    void SetAnimation(int animation_id)
    {
        m_sprite->SetAnimation(animation_id);
    }

    mono::ISpritePtr m_sprite;
    bool m_enabled = false;
};


Shuttle::Shuttle(const math::Vector& position, mono::EventHandler& eventHandler, const System::ControllerState& controller)
    : m_controller(this, eventHandler, controller),
      m_fire(false),
      m_player_info(nullptr)
{
    m_position = position;
    m_scale = math::Vector(1.0f, 1.0f);
    
    m_physics.body = mono::PhysicsFactory::CreateBody(10.0f, INFINITY);
    m_physics.body->SetPosition(m_position);

    mono::IShapePtr shape = mono::PhysicsFactory::CreateShape(m_physics.body, m_scale.x / 2.0f, math::ZeroVec);
    shape->SetElasticity(0.1f);
    shape->SetCollisionFilter(CollisionCategory::PLAYER, PLAYER_MASK);
    
    m_physics.shapes.push_back(shape);

    m_sprite = mono::CreateSprite("res/sprites/shuttle.sprite");
    m_sprite->SetAnimation(constants::IDLE);

    m_left_booster = std::make_shared<SpriteEntity>("res/sprites/booster.sprite");
    m_left_booster->SetScale(math::Vector(0.5f, 0.5f));
    m_left_booster->SetRotation(-math::PI_2());
    m_left_booster->SetPosition(math::Vector(-0.6f, 0.0f));

    m_right_booster = std::make_shared<SpriteEntity>("res/sprites/booster.sprite");
    m_right_booster->SetScale(math::Vector(0.5f, 0.5f));
    m_right_booster->SetRotation(math::PI_2());
    m_right_booster->SetPosition(math::Vector(0.6f, 0.0f));
    
    m_pool = std::make_unique<mono::ParticlePool>(1000, mono::DefaultUpdater);

    mono::ITexturePtr texture = mono::CreateTexture("res/textures/flare.png");
    m_particle_drawer = std::make_unique<mono::ParticleDrawer>(texture, *m_pool);

    AddChild(std::make_shared<game::TrailEffect>(m_position));
    AddChild(m_left_booster);
    AddChild(m_right_booster);

    SetProperty(EntityProperties::DAMAGABLE);
    
    // Make sure we have a weapon
    SelectWeapon(WeaponType::STANDARD);
}

Shuttle::~Shuttle()
{ }

#include <cstdio>

void Shuttle::Draw(mono::IRenderer& renderer) const
{
    renderer.DrawSprite(*m_sprite);

    renderer.PushGlobalTransform();
    m_particle_drawer->doDraw(renderer);

    //char text[32] = { '\0' };
    //std::snprintf(text, 32, "%zu", m_pool->m_count_alive);
    //constexpr mono::Color::RGBA color(1, 0, 0);
    //renderer.DrawText(0, text, math::ZeroVec, true, color);
}

void Shuttle::Update(unsigned int delta)
{
    m_controller.Update(delta);
    m_sprite->doUpdate(delta);
    m_pool->doUpdate(delta);

    if(m_fire)
        m_weapon->Fire(m_position, m_rotation);

    if(m_player_info)
    {
        m_player_info->position = m_position;
        m_player_info->ammunition_left = m_weapon->AmmunitionLeft();
        m_player_info->ammunition_capacity = m_weapon->MagazineSize();
        m_player_info->weapon_type = m_weapon_type;
    }
}

void Shuttle::SelectWeapon(WeaponType weapon)
{
    m_weapon = weapon_factory->CreateWeapon(weapon, WeaponFaction::PLAYER, m_pool.get());
    m_weapon_type = weapon;
}

void Shuttle::ApplyRotationForce(float force)
{
    const math::Vector forceVector(force, 0.0);

    // First apply the rotational force at an offset of 20 in y axis, then negate the vector
    // and apply it to zero to counter the movement when we only want rotation.
    m_physics.body->ApplyForce(forceVector, math::Vector(0, 20));
    m_physics.body->ApplyForce(forceVector * -1, math::ZeroVec);
}

void Shuttle::ApplyThrustForce(float force)
{
    const float rotation = Rotation();
    const math::Vector unit(-std::sin(rotation), std::cos(rotation));

    m_physics.body->ApplyForce(unit * force, math::ZeroVec);
}

void Shuttle::ApplyImpulse(const math::Vector& force)
{
    m_physics.body->ApplyImpulse(force, m_position);
}

void Shuttle::Fire()
{
    m_fire = true;
}

void Shuttle::StopFire()
{
    m_fire = false;
}

void Shuttle::Reload()
{
    m_weapon->Reload();
}

void Shuttle::SetBoosterThrusting(BoosterPosition position, bool enable)
{
    const int state = enable ? constants::THRUSTING : constants::IDLE;

    switch(position)
    {
        case BoosterPosition::LEFT:
            m_left_booster->SetAnimation(state);
            m_left_booster->m_enabled = enable;
            break;
        case BoosterPosition::RIGHT:
            m_right_booster->SetAnimation(state);
            m_right_booster->m_enabled = enable;
            break;
        case BoosterPosition::MAIN:
            m_sprite->SetAnimation(state);
            break;
        case BoosterPosition::ALL:
            m_left_booster->SetAnimation(state);
            m_left_booster->m_enabled = enable;
            m_right_booster->SetAnimation(state);
            m_right_booster->m_enabled = enable;
            m_sprite->SetAnimation(state);
            break;
    }
}

void Shuttle::SetShading(const mono::Color::RGBA& shade)
{
    m_sprite->SetShade(shade);
}

void Shuttle::SetPlayerInfo(PlayerInfo* info)
{
    m_player_info = info;
}
