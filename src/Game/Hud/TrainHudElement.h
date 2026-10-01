
#pragma once

#include "MonoFwd.h"
#include "UIElements.h"

namespace game
{
    class TrainHudElement : public UIOverlay
    {
    public:

        TrainHudElement();
        void Update(const mono::UpdateContext& context) override;

        // throttle is -1 (full reverse) to 1 (full forward)
        void SetThrottle(float throttle);
        void SetSpeed(float meters_per_second);
        void SetMoney(int chips, int rubble);

    private:

        float m_throttle;
        float m_speed;
        int m_chips;
        int m_rubble;

        class UIBarElement* m_throttle_bar;
        class UITextElement* m_speed_text;
        class UITextElement* m_chips_text;
        class UITextElement* m_rubble_text;
    };
}
