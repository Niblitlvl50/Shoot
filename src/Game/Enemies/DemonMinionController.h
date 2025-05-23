
#pragma once

#include "MonoFwd.h"
#include "Math/MathFwd.h"
#include "Physics/PhysicsFwd.h"
#include "Rendering/RenderFwd.h"
#include "StateMachine.h"
#include "System/Audio.h"

#include "Entity/IEntityLogic.h"
#include "Entity/TargetTypes.h"
#include "Weapons/WeaponFwd.h"

namespace game
{
    class DemonMinionController : public IEntityLogic
    {
    public:

        DemonMinionController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler* event_handler);
        ~DemonMinionController();

        void Update(const mono::UpdateContext& update_context) override;
        void DrawDebugInfo(class IDebugDrawer* debug_drawer) const override;
        const char* GetDebugCategory() const override;

        void UpdateMovement(const mono::UpdateContext& update_context);

        void OnIdle();
        void Idle(const mono::UpdateContext& update_context);

        void OnActive();
        void Active(const mono::UpdateContext& update_context);

        void OnTurn();
        void TurnToPlayer(const mono::UpdateContext& update_context);

        void OnCircleAttack();
        void CircleAttack(const mono::UpdateContext& update_context);

        void OnFireHoming();
        void ActionFireHoming(const mono::UpdateContext& update_context);

        void OnLongAttack();
        void ActionLongAttack(const mono::UpdateContext& update_context);

        void OnDead();
        void Dead(const mono::UpdateContext& update_context);

        void OnDamage(uint32_t who_did_damage, int damage);

    private:

        const uint32_t m_entity_id;
        mono::TransformSystem* m_transform_system;
        mono::PhysicsSystem* m_physics_system;
        class DamageSystem* m_damage_system;
        class TargetSystem* m_target_system;

        mono::IBody* m_entity_body;
        mono::ISprite* m_entity_sprite;
        uint32_t m_idle_animation;
        uint32_t m_turn_animation;
        uint32_t m_attack_animation;
        uint32_t m_death_animation;

        ITargetPtr m_aquired_target;

        bool m_ready_to_attack;
        float m_circle_attack_cooldown;
        float m_fire_homing_cooldown;
        float m_long_attack_cooldown;

        IWeaponPtr m_primary_weapon;
        IWeaponPtr m_secondary_weapon;
        IWeaponPtr m_tertiary_weapon;

        audio::ISoundPtr m_damage_sound;
        audio::ISoundPtr m_death_sound;

        enum class States
        {
            IDLE,
            ACTIVE,
            TURN_TO_PLAYER,
            ACTION_FIRE_CIRCLE,
            ACTION_FIRE_HOMING,
            ACTION_FIRE_LONG,
            DEAD,
        };

        const char* StateToString(States state) const;
        void TurnAndTransitionTo(States state_after_turn);

        using CacoStateMachine = StateMachine<States, const mono::UpdateContext&>;
        CacoStateMachine m_states;

        States m_state_after_turn;
        bool m_beast_mode;
    };
}
