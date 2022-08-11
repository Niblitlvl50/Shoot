
#pragma once

#include "MonoFwd.h"
#include "Math/MathFwd.h"
#include "Physics/PhysicsFwd.h"
#include "Rendering/RenderFwd.h"
#include "StateMachine.h"

#include "Entity/IEntityLogic.h"
#include "Weapons/WeaponFwd.h"

#include <memory>

namespace game
{
    class CacodemonController : public IEntityLogic
    {
    public:

        CacodemonController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler& event_handler);
        ~CacodemonController();

        void Update(const mono::UpdateContext& update_context) override;
        void DrawDebugInfo(class IDebugDrawer* debug_drawer) const override;
        const char* GetDebugCategory() const override;

        void OnIdle();
        void Idle(const mono::UpdateContext& update_context);

        void OnActive();
        void Active(const mono::UpdateContext& update_context);

        void OnAction();
        void ActionShockwave(const mono::UpdateContext& update_context);
        void ActionFireHoming(const mono::UpdateContext& update_context);
        void ActionFireBeam(const mono::UpdateContext& update_context);

        void OnDead();
        void Dead(const mono::UpdateContext& update_context);

    private:

        const uint32_t m_entity_id;
        mono::TransformSystem* m_transform_system;
        mono::PhysicsSystem* m_physics_system;

        mono::IBody* m_entity_body;
        mono::ISprite* m_entity_sprite;
        uint32_t m_idle_animation;
        uint32_t m_attack_animation;
        uint32_t m_death_animation;

        std::unique_ptr<class ShockwaveEffect> m_shockwave_effect;

        const struct PlayerInfo* m_target_player;
        bool m_ready_to_attack;
        float m_shockwave_cooldown;
        float m_fire_homing_cooldown;
        float m_fire_beam_cooldown;

        IWeaponPtr m_primary_weapon;
        IWeaponPtr m_secondary_weapon;

        enum class States
        {
            IDLE,
            ACTIVE,
            ACTION_SHOCKWAVE,
            ACTION_FIRE_HOMING,
            ACTION_FIRE_BEAM,
            DEAD,
        };

        using CacoStateMachine = StateMachine<States, const mono::UpdateContext&>;
        CacoStateMachine m_states;
    };
}
