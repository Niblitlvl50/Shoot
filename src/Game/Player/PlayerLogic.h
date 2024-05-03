
#pragma once

#include "MonoFwd.h"
#include "Math/Vector.h"
#include "StateMachine.h"
#include "System/Audio.h"
#include "Physics/PhysicsFwd.h"

#include "Entity/IEntityLogic.h"
#include "PlayerAbilities.h"
#include "PlayerConfig.h"
#include "Controllers/PlayerGamepadController.h"
#include "Controllers/PlayerKeybordController.h"
#include "Weapons/WeaponTypes.h"
#include "Weapons/WeaponFwd.h"
#include "Weapons/WeaponModifierTypes.h"
#include "Pickups/PickupTypes.h"

#include <memory>

namespace game
{
    struct PlayerInfo;

    enum ItemSlotIndex
    {
        LEFT,
        RIGHT,
        N_SLOTS
    };

    constexpr int N_WEAPONS = 3;

    class PlayerLogic : public IEntityLogic
    {
    public:

        PlayerLogic(
            uint32_t entity_id,
            PlayerInfo* player_info,
            const PlayerConfig& config,
            mono::InputSystem* input_system,
            mono::EventHandler* event_handler,
            mono::SystemContext* system_context);

        ~PlayerLogic();

        void DrawDebugInfo(class IDebugDrawer* debug_drawer) const override;
        const char* GetDebugCategory() const override;
        void Update(const mono::UpdateContext& update_context) override;

        void UpdatePlayerInfo(uint32_t timestamp);
        void UpdateMovement(const mono::UpdateContext& update_context);
        void UpdateAnimation(const mono::UpdateContext& update_context, float aim_direction, const math::Vector& world_position, const math::Vector& player_velocity);
        void UpdateController(const mono::UpdateContext& update_context);

        void Fire(uint32_t timestamp);
        void StopFire();
        void Reload(uint32_t timestamp);
        void UseItemSlot(ItemSlotIndex slot_index);
        void HandlePickup(PickupType type, int amount);

        void CycleWeapon();
        void AddDamageBuff(int amount);

        void TriggerHookshot();
        void ReleaseHookshot();

        void Throw(float throw_force);
        void ThrowAction();
        void PickupDrop();
        bool HoldingPickup() const; 

        void Sprint();
        void StopSprint();
        bool HasStamina() const;

        void MoveInDirection(const math::Vector& direction);
        void ResetMovement();

        void ApplyImpulse(const math::Vector& force);
        void ApplyForce(const math::Vector& force);
        void SetVelocity(const math::Vector& velocity);

        void SetAimDirection(float aim_direction);
        void SetAimScreenPosition(const math::Vector& aim_screen_position);
        void Blink(const math::Vector& direction);
        void Shockwave();
        void Shield();

        void RespawnPlayer();
        void TogglePauseGame();

        void ToDefault();
        void DefaultState(const mono::UpdateContext& update_context);

        void ToDead();
        void DeadState(const mono::UpdateContext& update_context);
        void ExitDead();

        void ToBlink();
        void BlinkState(const mono::UpdateContext& update_context);
        void ExitBlink();

        const uint32_t m_entity_id;
        PlayerInfo* m_player_info;
        PlayerConfig m_config;
        PlayerGamepadController m_gamepad_controller;
        PlayerKeyboardController m_keyboard_controller;
        mono::EventHandler* m_event_handler;
        bool m_pause;

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
        bool m_stop_fire;
        float m_aim_direction;
        float m_aim_target;
        float m_aim_velocity;
        math::Vector m_aim_screen_position;

        bool m_sprint;
        float m_stamina;
        float m_stamina_recover_timer_s;

        int m_weapon_index;
        IWeaponPtr m_weapons[N_WEAPONS];
        IWeaponPtr m_familiar_weapon;

        int m_idle_anim_id;
        int m_run_anim_id;
        int m_run_up_anim_id;
        int m_death_anim_id;

        float m_blink_duration_counter;
        math::Vector m_blink_direction;
        std::unique_ptr<class SmokeEffect> m_smoke_effect;
        std::unique_ptr<class ShockwaveEffect> m_shockwave_effect;
        std::unique_ptr<class FootStepsEffect> m_footsteps_effect;
        std::unique_ptr<class WeaponModifierEffect> m_weapon_modifier_effect;
        audio::ISoundPtr m_switch_weapon_sound;
        audio::ISoundPtr m_blink_sound;
        audio::ISoundPtr m_running_sound;

        math::Vector m_movement_direction;
        float m_accumulated_step_distance;

        float m_blink_cooldown;
        float m_shockwave_cooldown;
        float m_shield_cooldown;

        mono::TransformSystem* m_transform_system;
        mono::InputSystem* m_input_system;
        mono::PhysicsSystem* m_physics_system;
        mono::RenderSystem* m_render_system;
        mono::SpriteSystem* m_sprite_system;
        mono::LightSystem* m_light_system;
        mono::IEntityManager* m_entity_system;
        class DamageSystem* m_damage_system;
        class PickupSystem* m_pickup_system;
        class InteractionSystem* m_interaction_system;
        class EntityLogicSystem* m_logic_system;
        class WeaponSystem* m_weapon_system;
        class TargetSystem* m_target_system;

        mono::InputContext* m_input_context;

        //struct ItemSlot
        //{};
        //ItemSlot m_item_slots[ItemSlotIndex::N_SLOTS];

        float m_active_cooldowns[PlayerAbility::N_ABILITIES];
        float m_active_weapon_modifiers[WeaponModifier::N_MODIFIERS];

        int m_damage_modifier_handle;

        uint32_t m_picked_up_id;
        mono::IConstraint* m_pickup_constraint;
        float m_pickup_mass;

        std::unique_ptr<class Hookshot> m_hookshot;
    };
}
