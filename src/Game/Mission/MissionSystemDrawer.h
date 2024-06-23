
#pragma once

#include "Hud/UIElements.h"

#include <cstdint>
#include <vector>

namespace game
{
    class MissionSystemDrawer : public game::UIOverlay
    {
    public:

        MissionSystemDrawer(const class MissionSystem* mission_system);
        void Update(const mono::UpdateContext& context) override;

    private:

        void UpdateAnimations(const mono::UpdateContext& context);

        class MissionStatusUIElement* AddMissionUIElement(uint32_t entity_id);
        void RemoveMissionUIElement(uint32_t entity_id);
        void CompleteAndRemoveMissionUIElement(uint32_t entity_id);
        void FailAndRemoveMissionUIElement(uint32_t entity_id);
        void ReCalculateLayout();
        math::Vector CalculatePositionForItem(int item_index) const;

        const MissionSystem* m_mission_system;

        struct MissionStatusData
        {
            uint32_t entity_id;
            math::Vector desired_position;
            math::Vector current_velocity;
            class MissionStatusUIElement* ui_element;
        };
        std::vector<MissionStatusData> m_mission_ui_collection;
    };
}
