
#pragma once

#include "Hud/UIElements.h"

namespace game
{
    class HordeWaveDrawer : public UIOverlay
    {
    public:

        HordeWaveDrawer();
        void Update(const mono::UpdateContext& update_context) override;

        void ShowNextWave(int wave_index, const std::string& sub_text);
    
    private:

        math::Vector m_onscreen_position;
        math::Vector m_offscreen_position;

        UISquareElement* m_background;
        UITextElement* m_wave_text;
        UITextElement* m_wave_subtext;
        float m_text_timer_s;
    };
}
