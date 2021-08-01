
#pragma once

#include "MonoFwd.h"
#include "Math/Vector.h"
#include "StateMachine.h"
#include "System/Audio.h"

#include "Entity/IEntityLogic.h"
#include "PlayerGamepadController.h"
#include "Weapons/WeaponTypes.h"
#include "Weapons/IWeaponFactory.h"

#include <memory>

namespace game
{
    struct PlayerInfo;

    enum class BlinkDirection
    {
        LEFT,
        RIGHT,
        UP,
        DOWN
    };

    class PlayerLogic : public IEntityLogic
    {
    public:

        PlayerLogic(
            uint32_t entity_id,
            PlayerInfo* player_info,
            mono::EventHandler* event_handler,
            const System::ControllerState& controller,
            mono::SystemContext* system_context);

        ~PlayerLogic();

        void Update(const mono::UpdateContext& update_context) override;
        void UpdateAnimation(float aim_direction, const math::Vector& player_velocity);

        void Fire();
        void StopFire();
        void Reload(uint32_t);

        void SecondaryFire();

        void TriggerInteraction();

        void SelectWeapon(WeaponType weapon);
        void SelectSecondaryWeapon(WeaponType weapon);
        
        void MoveInDirection(const math::Vector& direction);
        void ApplyImpulse(const math::Vector& force);
        void ApplyForce(const math::Vector& force);
        void SetVelocity(const math::Vector& velocity);
        void ResetForces();

        void SetAimDirection(float aim_direction);
        void Blink(BlinkDirection direction);

        void ToDefault();
        void DefaultState(const mono::UpdateContext& update_context);

        void ToDead();
        void DeadState(const mono::UpdateContext& update_context);

        void ToBlink();
        void BlinkState(const mono::UpdateContext& update_context);

        const uint32_t m_entity_id;
        const uint32_t m_controller_id;
        PlayerInfo* m_player_info;
        PlayerGamepadController m_gamepad_controller;

        enum class PlayerStates
        {
            DEFAULT,
            DEAD,
            BLINK
        };

        using PlayerStateMachine = StateMachine<PlayerStates, const mono::UpdateContext&>;
        PlayerStateMachine m_state;

        uint32_t m_weapon_entity;

        bool m_fire;
        bool m_secondary_fire;
        int m_total_ammo_left;
        WeaponType m_weapon_type;
        IWeaponPtr m_weapon;
        IWeaponPtr m_secondary_weapon;
        float m_aim_direction;

        int m_idle_anim_id;
        int m_run_anim_id;

        uint32_t m_blink_counter;
        BlinkDirection m_blink_direction;

        std::unique_ptr<class TrailEffect> m_trail_effect;
        std::unique_ptr<class BlinkEffect> m_blink_effect;

        audio::ISoundPtr m_blink_sound;

        mono::TransformSystem* m_transform_system;
        mono::PhysicsSystem* m_physics_system;
        mono::SpriteSystem* m_sprite_system;
        mono::IEntityManager* m_entity_system;
        class PickupSystem* m_pickup_system;
        class InteractionSystem* m_interaction_system;
    };
}
