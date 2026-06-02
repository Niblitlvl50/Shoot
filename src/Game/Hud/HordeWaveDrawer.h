
#pragma once

#include "Hud/UIElements.h"

namespace game
{
    class HordeWaveInfoElement : public UISquareElement
    {
    public:

        HordeWaveInfoElement();
        void SetWaveInfo(int wave_index, const std::string& sub_text, const std::string& sub_text_description, const std::string& sprite_file);

    private:

        UITextElement* m_wave_text;
        UITextElement* m_wave_subtext;
        UITextElement* m_wave_subtext_description;
        UISpriteElement* m_icon;
    };

    class HordeWaveDrawer : public UIOverlay
    {
    public:

        HordeWaveDrawer();
        void Update(const mono::UpdateContext& update_context) override;
        void ShowNextWave(int wave_index, const std::string& sub_text, const std::string& sub_text_description, const std::string& sprite_file);
    
    private:

        math::Vector m_onscreen_position;
        math::Vector m_offscreen_position;
        HordeWaveInfoElement* m_wave_info_element;
        float m_text_timer_s;
    };
}
