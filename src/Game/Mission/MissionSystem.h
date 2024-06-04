
#pragma once

#include "MonoFwd.h"
#include "IGameSystem.h"

#include <vector>
#include <string>
#include <cstdint>

namespace game
{
    enum class MissionType
    {
        Kill,
        Defend,
        Deliver
    };

    struct Mission
    {
        float value;
        MissionType type;
        std::string name;
        std::string entity_collection;
    };

    struct ActivatedMission
    {
        std::vector<uint32_t> spawned_entities;
    };

    class MissionSystem : public mono::IGameSystem
    {
    public:

        MissionSystem(mono::IEntityManager* entity_manager, mono::TransformSystem* transform_system);
        const char* Name() const override;
        void Update(const mono::UpdateContext& update_context) override;

        void InitializeMissionPositions(const std::vector<uint32_t>& mission_points);
        void ActivateMission();

    private:

        mono::IEntityManager* m_entity_manager;
        mono::TransformSystem* m_transform_system;

        std::vector<uint32_t> m_mission_points;
        std::vector<Mission> m_defined_missions;

        uint32_t m_point_index;

        ActivatedMission m_activated_mission;
    };
}
