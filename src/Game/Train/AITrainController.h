
#pragma once

#include "MonoFwd.h"
#include "Entity/IEntityLogic.h"
#include "StateMachine.h"
#include "Math/Vector.h"

#include <cstdint>

namespace game
{
    // Drives a train around the railway network on its own. The entity needs a manually controlled
    // path_follower and a train_car component with is_locomotive set, like the player's train. Switches
    // are taken as they're set, the railway system hands the train across them like any other train.
    //
    // It cruises along, brakes for and waits at stations, stops and reverses at dead ends, and stops when
    // another train is on the track just ahead. Everything ahead is measured in world space along the
    // direction of travel, positions along a track can count backwards after a switch.
    class AITrainController : public IEntityLogic
    {
    public:

        AITrainController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler* event_handler);

        void Update(const mono::UpdateContext& update_context) override;
        void DrawDebugInfo(class IDebugDrawer* debug_drawer) const override;
        const char* GetDebugCategory() const override;

    private:

        void ToDriving();
        void Driving(const mono::UpdateContext& update_context);

        void ToStoppedAtStation();
        void StoppedAtStation(const mono::UpdateContext& update_context);

        void ToDeadEnd();
        void DeadEnd(const mono::UpdateContext& update_context);

        void ToBlocked();
        void Blocked(const mono::UpdateContext& update_context);

        // Eases the throttle towards target_throttle (0 - 1) and applies it in the direction of travel.
        void ApplyThrottle(float target_throttle, float delta_s);

        math::Vector TravelDirection() const;
        float DistanceToTrackEnd() const;
        bool IsAtTrackEnd() const;

        // Distance along the direction of travel to the closest station on the track ahead, or a negative
        // value if there's none. The last station the train stopped at is ignored until it's left behind.
        float DistanceToStationAhead(uint32_t& out_station_id) const;

        // Distance to the closest other train (any car not in this train's chain) on the track ahead, or a
        // negative value if there's none.
        float DistanceToTrainAhead() const;

        // Near the end of a track that leads into a switch from its trunk, decides once per approach
        // whether to toggle the switch and take the other branch.
        void DecideBranchAhead();

        enum class States { DRIVING, STOPPED_AT_STATION, DEAD_END, BLOCKED };

        const uint32_t m_entity_id;
        mono::TransformSystem* m_transform_system;
        class PathFollowerSystem* m_path_follower_system;
        class RailwaySystem* m_railway_system;
        class TrainCarSystem* m_train_car_system;

        using TrainStateMachine = StateMachine<States, const mono::UpdateContext&>;
        TrainStateMachine m_states;

        // +1 or -1, the sign of the throttle. A switch hand-off keeps the meaning of the sign.
        float m_direction;
        float m_throttle;
        float m_state_timer_s;

        uint32_t m_target_station_id;
        uint32_t m_last_station_id;

        // The track the branch decision was made on, so it's only made once per approach.
        uint32_t m_branch_decided_on_track_id;
    };
}
