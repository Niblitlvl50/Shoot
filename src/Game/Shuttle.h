
#pragma once

#include "MonoPtrFwd.h"
#include "Entity/PhysicsEntityBase.h"
#include "Rendering/RenderPtrFwd.h"
#include "ShuttleGamepadController.h"
#include "Physics/IBody.h"

#include "Weapons/IWeaponSystem.h"

#include "System/System.h"

class SpriteEntity;

namespace game
{
    enum class WeaponType;

    enum class BoosterPosition
    {
        LEFT,
        RIGHT,
        MAIN,
        ALL
    };

    class Shuttle : public mono::PhysicsEntityBase, public mono::ICollisionHandler
    {
    public:
        
        Shuttle(
            const math::Vector& postiion, mono::EventHandler& eventHandler, const System::ControllerState& controller);

        void SelectWeapon(WeaponType weapon);
        void ApplyImpulse(const math::Vector& force);

        void Fire();
        void StopFire();

        void SetBoosterThrusting(BoosterPosition position, bool enabled);

    private:
        
        void ApplyRotationForce(float force);
        void ApplyThrustForce(float force);

        virtual void Draw(mono::IRenderer& renderer) const;
        virtual void Update(unsigned int delta);
        
        virtual void OnCollideWith(const mono::IBodyPtr& body);
        virtual void OnPostStep();
        
        ShuttleGamepadController m_controller;
        bool m_fire;
        
        mono::ISpritePtr m_sprite;
        std::unique_ptr<IWeaponSystem> m_weapon;
        std::shared_ptr<SpriteEntity> m_left_booster;
        std::shared_ptr<SpriteEntity> m_right_booster;
    };
}

