
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
        UITextElement* m_mission_name_text;
        UITextElement* m_mission_description_text;

        float m_text_timer_s;
    };
}
