
#pragma once

#include "Hud/UIElements.h"
#include <cstdint>

namespace game
{
    class MissionSystemDrawer : public game::UIOverlay
    {
    public:

        MissionSystemDrawer(class MissionSystem* mission_system);
        void Update(const mono::UpdateContext& context) override;

    private:

        MissionSystem* m_mission_system;

        UISquareElement* m_background;
        UITextElement* m_mission_name_text;
        UITextElement* m_mission_description_text;

        math::Vector m_onscreen_position;
        math::Vector m_offscreen_position;
        float m_text_timer_s;
    };
}
