
#pragma once

#include "MonoFwd.h"
#include "Rendering/RenderFwd.h"
#include "Entity/IEntityLogic.h"
#include "Entity/TargetTypes.h"
#include "Math/Vector.h"
#include "Math/Interval.h"
#include "StateMachine.h"

#include "Behaviour/TrackingBehaviour.h"

#include <vector>

namespace game
{
    struct WanderSettings
    {
        // The path entity whose points it walks between.
        uint32_t path_entity_uuid;

        // Seconds to stand still before walking to a new point.
        math::Interval pause_interval;

        // Walks up to a player within this distance, zero turns it off.
        float approach_distance;
        float speed;
    };

    // Walks to a random point of a path, pauses, and picks another point. Only the path's points are
    // used, the walking goes through the navmesh. When a player comes within the approach distance it
    // walks up to where the player was, once, and stays there until the player leaves.
    class WanderController : public IEntityLogic
    {
    public:

        WanderController(uint32_t entity_id, const WanderSettings& settings, mono::SystemContext* system_context);

        void Update(const mono::UpdateContext& update_context) override;
        void DrawDebugInfo(class IDebugDrawer* debug_drawer) const override;
        const char* GetDebugCategory() const override;

    private:

        enum class States
        {
            PAUSE,
            WANDER,
            APPROACH,
        };

        void ToPause();
        void PauseState(const mono::UpdateContext& update_context);

        void ToWander();
        void WanderState(const mono::UpdateContext& update_context);

        void ToApproach();
        void ApproachState(const mono::UpdateContext& update_context);

        void StandStill();
        void PlayAnimation(const char* animation_name);
        void ReadWanderPoints();

        const uint32_t m_entity_id;
        const WanderSettings m_settings;

        mono::TransformSystem* m_transform_system;
        mono::IEntityManager* m_entity_manager;
        mono::PathSystem* m_path_system;
        mono::IBody* m_body;
        mono::ISprite* m_sprite;
        const char* m_animation_name;
        class TargetSystem* m_target_system;

        std::vector<math::Vector> m_wander_points;
        int m_current_point_index;
        bool m_approach_done;

        float m_pause_time_s;
        float m_target_check_timer_s;
        ITargetPtr m_target;

        using MyStateMachine = StateMachine<States, const mono::UpdateContext&>;
        MyStateMachine m_states;

        TrackingBehaviour m_tracking_behaviour;
    };
}
