
#pragma once

#include "MonoFwd.h"
#include "IGameSystem.h"

#include <unordered_map>
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
        uint32_t entity_id;

        std::string name;
        std::string description;
        uint32_t activated_trigger;
        uint32_t completed_trigger;
        uint32_t failed_trigger;

        MissionStatus status;

        // Internal data
        uint32_t activated_callback_id;
        uint32_t completed_callback_id;
        uint32_t failed_callback_id;
    };

    class MissionSystem : public mono::IGameSystem
    {
    public:

        MissionSystem(mono::IEntityManager* entity_manager, mono::TransformSystem* transform_system, game::TriggerSystem* trigger_system);
        const char* Name() const override;
        void Begin() override;
        void Update(const mono::UpdateContext& update_context) override;

        void InitializeMissionPositions(const std::vector<uint32_t>& mission_points);
        void ActivateMission();
        const std::vector<MissionTrackerComponent>& GetMissionStatus() const;

        MissionTrackerComponent* AllocateMission(uint32_t entity_id);
        void ReleaseMission(uint32_t entity_id);
        void SetMissionData(
            uint32_t entity_id, const std::string& name, const std::string& description, uint32_t activated_trigger_hash, uint32_t completed_trigger_hash, uint32_t failed_trigger_hash);

    private:

        void HandleMissionActivated(uint32_t entity_id);
        void HandleMissionCompleted(uint32_t entity_id);
        void HandleMissionFailed(uint32_t entity_id);

        mono::IEntityManager* m_entity_manager;
        mono::TransformSystem* m_transform_system;
        game::TriggerSystem* m_trigger_system;

        uint32_t m_point_index;
        std::vector<uint32_t> m_mission_points;
        std::vector<std::string> m_spawnable_missions;

        std::unordered_map<uint32_t, MissionTrackerComponent> m_mission_trackers;
    };
}
