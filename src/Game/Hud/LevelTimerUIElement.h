
#pragma once

#include "MonoFwd.h"
#include "UIElements.h"

namespace game
{
    class LevelTimerUIElement : public UIOverlay
    {
    public:

        LevelTimerUIElement();
        void Update(const mono::UpdateContext& context) override;
        void SetSeconds(int seconds);

    private:
        int m_seconds;
        class UITextElement* m_time_element;
    };
}
