
#pragma once

#include "Entity/IEntityLogic.h"
#include "ShuttleGamepadController.h"
#include "PlayerInteractionController.h"
#include "Weapons/IWeaponSystem.h"
#include "Weapons/WeaponTypes.h"

#include "MonoFwd.h"

namespace mono
{
    class TransformSystem;
    class PhysicsSystem;
}

namespace game
{
    struct PlayerInfo;

    class ShuttleLogic : public IEntityLogic
    {
    public:

        ShuttleLogic(
            uint32_t entity_id,
            PlayerInfo* player_info,
            mono::EventHandler& event_handler,
            const System::ControllerState& controller,
            mono::SystemContext* system_context);

        void Update(uint32_t delta_ms) override;

        void Fire();
        void StopFire();
        void Reload();
        void SelectWeapon(WeaponType weapon);
        void ApplyImpulse(const math::Vector& force);
        void SetRotation(float rotation);
        void GiveAmmo(int value);
        void GiveHealth(int value);
        uint32_t EntityId() const;

        const uint32_t m_entity_id;
        PlayerInfo* m_player_info;
        ShuttleGamepadController m_gamepad_controller;
        PlayerInteractionController m_interaction_controller;

        bool m_fire;
        int m_total_ammo_left;
        std::unique_ptr<IWeaponSystem> m_weapon;
        WeaponType m_weapon_type;

        mono::TransformSystem* m_transform_system;
        mono::PhysicsSystem* m_physics_system;
    };
}
