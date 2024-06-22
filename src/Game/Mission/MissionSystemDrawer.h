
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

        class MissionStatusUIElement* AddMissionUIElement(uint32_t entity_id);
        void RemoveMissionUIElement(uint32_t entity_id);
        void ReCalculateLayout();

        const MissionSystem* m_mission_system;

        struct MissionStatusData
        {
            uint32_t entity_id;
            float timer_s;
            math::Vector onscreen_position;
            math::Vector offscreen_position;
            class MissionStatusUIElement* ui_element;
        };
        std::vector<MissionStatusData> m_mission_ui_collection;
    };
}
