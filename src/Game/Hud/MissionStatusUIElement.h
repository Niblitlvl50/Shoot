
#pragma once

#include "Hud/UIElements.h"

namespace game
{
    class MissionStatusUIElement : public game::UISquareElement
    {
    public:

        MissionStatusUIElement(float width, float height, const mono::Color::RGBA& background_color);
        void SetText(const std::string& text);
        void SetDescription(const std::string& description);
        void ShowIcon(bool show, int index);
        void ShowTimer(bool show);
        void SetTime(float time_s, float max_time_s);

        UITextElement* m_mission_name_text;
        UITextElement* m_mission_description_text;
        UISpriteElement* m_icon;
        UIBarElement* m_timer_bar;
    };
}
