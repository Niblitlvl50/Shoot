
#pragma once

#include "MonoFwd.h"
#include "Math/Vector.h"
#include "StateMachine.h"
#include "System/Audio.h"
#include "Physics/PhysicsFwd.h"

#include "Entity/IEntityLogic.h"
#include "PlayerAbilities.h"
#include "PlayerConfig.h"
#include "Pickups/PickupTypes.h"

#include <memory>

namespace game
{
    struct PlayerInfo;

    class TrainLogic : public IEntityLogic
    {
    public:

        TrainLogic(
            uint32_t entity_id,
            PlayerInfo* player_info,
            const PlayerConfig& config,
            mono::InputSystem* input_system,
            mono::EventHandler* event_handler,
            mono::SystemContext* system_context);

        ~TrainLogic();

        void DrawDebugInfo(class IDebugDrawer* debug_drawer) const override;
        const char* GetDebugCategory() const override;
        void Update(const mono::UpdateContext& update_context) override;

        void UpdateAutoAim();
        void UpdatePlayerInfo(uint32_t timestamp);
        void UpdateMovement(const mono::UpdateContext& update_context);
        void UpdateAnimation(const mono::UpdateContext& update_context, float aim_direction, const math::Vector& world_position, const math::Vector& player_velocity);
        void UpdateController(const mono::UpdateContext& update_context);

        void HandlePickup(PickupType type, int meta_data);

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

        void ApplyImpulse(const math::Vector& force);
        void ApplyForce(const math::Vector& force);
        void SetVelocity(const math::Vector& velocity);

        void SetAimDirection(float aim_direction);
        void SetAimScreenPosition(const math::Vector& aim_screen_position);

        void RespawnPlayer();
        void TogglePauseGame();

        void ToDefault();
        void DefaultState(const mono::UpdateContext& update_context);

        void ToDead();
        void DeadState(const mono::UpdateContext& update_context);
        void ExitDead();

        const uint32_t m_entity_id;
        PlayerInfo* m_player_info;
        PlayerConfig m_config;
        //PlayerGamepadController m_gamepad_controller;
        //PlayerKeyboardController m_keyboard_controller;
        mono::EventHandler* m_event_handler;
        bool m_pause;

        enum class TrainStates
        {
            DEFAULT,
            DEAD,
        };

        using TrainStateMachine = StateMachine<TrainStates, const mono::UpdateContext&>;
        TrainStateMachine m_state;

        float m_aim_direction;
        float m_aim_target;
        float m_aim_velocity;
        math::Vector m_aim_screen_position;

        bool m_sprint;
        float m_stamina;
        float m_stamina_recover_timer_s;

        int m_idle_anim_id;
        int m_run_anim_id;
        int m_run_up_anim_id;
        int m_death_anim_id;

        std::unique_ptr<class SmokeEffect> m_smoke_effect;

        audio::ISoundPtr m_drop_box_sound;
        audio::ISoundPtr m_pickup_box_sound;
        audio::ISoundPtr m_running_sounds[2];

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
        class TargetSystem* m_target_system;

        mono::InputContext* m_input_context;

        uint32_t m_picked_up_id;
        mono::IConstraint* m_pickup_constraint;
        float m_pickup_mass;

        std::unique_ptr<class Hookshot> m_hookshot;
    };
}
