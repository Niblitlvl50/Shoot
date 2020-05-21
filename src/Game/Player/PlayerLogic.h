
#pragma once

#include "Entity/IEntityLogic.h"
#include "PlayerGamepadController.h"
#include "PlayerInteractionController.h"
#include "Weapons/IWeaponSystem.h"
#include "Weapons/WeaponTypes.h"
#include "Math/Vector.h"

#include "MonoFwd.h"

#include <memory>

namespace mono
{
    class TransformSystem;
    class PhysicsSystem;
    class SpriteSystem;
}

namespace game
{
    struct PlayerInfo;

    enum class PlayerAnimation
    {
        IDLE,
        DUCK,
        WALK_LEFT,
        WALK_RIGHT,
        WALK_UP
    };

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
            mono::EventHandler& event_handler,
            const System::ControllerState& controller,
            mono::SystemContext* system_context);

        ~PlayerLogic();

        void Update(const mono::UpdateContext& update_context) override;

        void Fire();
        void StopFire();
        void Reload();
        void SelectWeapon(WeaponType weapon);
        void ApplyImpulse(const math::Vector& force);
        void SetRotation(float rotation);
        void SetAnimation(PlayerAnimation animation);
        void GiveAmmo(int value);
        void GiveHealth(int value);
        void Blink(BlinkDirection direction);

        uint32_t EntityId() const;

        const uint32_t m_entity_id;
        uint32_t m_weapon_entity_id;
        uint32_t m_weapon_fire_offset_entity_id;
        
        PlayerInfo* m_player_info;
        PlayerGamepadController m_gamepad_controller;
        PlayerInteractionController m_interaction_controller;

        bool m_fire;
        int m_total_ammo_left;
        std::unique_ptr<IWeaponSystem> m_weapon;
        WeaponType m_weapon_type;
        float m_aim_direction;

        std::unique_ptr<class TrailEffect> m_trail_effect;

        mono::TransformSystem* m_transform_system;
        mono::PhysicsSystem* m_physics_system;
        mono::SpriteSystem* m_sprite_system;
    };
}
