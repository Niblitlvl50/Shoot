
#pragma once

#include "MonoFwd.h"
#include "MonoPtrFwd.h"
#include "Math/Vector.h"
#include "Entity/IEntityLogic.h"
#include "Entity/TargetTypes.h"
#include "Rendering/RenderFwd.h"
#include "Physics/PhysicsFwd.h"
#include "Weapons/WeaponFwd.h"
#include "StateMachine.h"

#include "Behaviour/PathBehaviour.h"

namespace game
{
    class InvaderPathController : public IEntityLogic
    {
    public:

        InvaderPathController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler* event_handler);
        ~InvaderPathController();

        void Update(const mono::UpdateContext& update_context) override;
        void DrawDebugInfo(class IDebugDrawer* debug_drawer) const override;
        const char* GetDebugCategory() const override;

    private:

        void ToFindPath();
        void FindPath(const mono::UpdateContext& update_context);

        void ToFollowPath();
        void FollowPath(const mono::UpdateContext& update_context);

        enum class States
        {
            FIND_PATH,
            FOLLOW_PATH,
        };

        using InvaderStateMachine = StateMachine<States, const mono::UpdateContext&>;
        InvaderStateMachine m_states;

        uint32_t m_entity_id;
        math::Vector m_spawn_position;

        int m_fire_count;
        float m_fire_cooldown_s;

        PathBehaviour m_path_behaviour;
        IWeaponPtr m_weapon;

        mono::ISprite* m_sprite;
        mono::IBody* m_body;

        mono::TransformSystem* m_transform_system;
        mono::PathSystem* m_path_system;
        class TargetSystem* m_target_system;
        ITargetPtr m_aquired_target;
    };
}
