
#pragma once

#include "MonoFwd.h"
#include "IGameSystem.h"

#include <vector>
#include <string>
#include <cstdint>

namespace game
{
    class TriggerSystem;

    enum class MissionStatus
    {
        Inactive,
        Active,
        Completed,
        Failed
    };

    struct MissionTrackerComponent
    {
        std::string name;
        std::string description;

        bool time_based;
        float time_s;
        bool fail_on_timeout;

        uint32_t activated_trigger;
        uint32_t completed_trigger;
        uint32_t failed_trigger;

        // Internal data
        MissionStatus status;
        uint32_t entity_id;
        uint32_t activated_callback_id;
        uint32_t completed_callback_id;
        uint32_t failed_callback_id;
    };

    struct MissionStatusEvent
    {
        uint32_t mission_id;
        MissionStatus status;
    };

    class MissionSystem : public mono::IGameSystem
    {
    public:

        MissionSystem(mono::IEntityManager* entity_manager, mono::TransformSystem* transform_system, game::TriggerSystem* trigger_system);
        const char* Name() const override;
        void Begin() override;
        void Sync() override;
        void Update(const mono::UpdateContext& update_context) override;

        void InitializeMissionPositions(const std::vector<uint32_t>& mission_points);
        void ActivateMission();
        const std::vector<MissionStatusEvent>& GetMissionStatusEvents() const;

        void AllocateMission(uint32_t entity_id);
        void ReleaseMission(uint32_t entity_id);
        void SetMissionData(
            uint32_t entity_id,
            const std::string& name,
            const std::string& description,
            bool time_based,
            float time_s,
            bool fail_on_timeout,
            uint32_t activated_trigger_hash,
            uint32_t completed_trigger_hash,
            uint32_t failed_trigger_hash);

        const MissionTrackerComponent* GetComponentById(uint32_t entity_id) const;

    private:

        MissionTrackerComponent* GetComponentById(uint32_t entity_id);

        void HandleMissionActivated(uint32_t entity_id);
        void HandleMissionCompleted(uint32_t entity_id);
        void HandleMissionFailed(uint32_t entity_id);

        mono::IEntityManager* m_entity_manager;
        mono::TransformSystem* m_transform_system;
        game::TriggerSystem* m_trigger_system;

        uint32_t m_point_index;
        std::vector<uint32_t> m_mission_points;
        std::vector<std::string> m_spawnable_missions;

        std::vector<MissionTrackerComponent> m_mission_trackers;
        std::vector<MissionStatusEvent> m_mission_status_events;
    };
}
