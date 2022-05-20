
#pragma once

#include "MonoFwd.h"
#include "Math/Vector.h"
#include "Rendering/RenderFwd.h"
#include "UIElements.h"

namespace game
{
    class BigTextScreen : public game::UIOverlay
    {
    public:

        BigTextScreen(
            const char* main_text,
            const char* sub_text,
            const mono::Color::RGBA& color,
            const mono::Color::RGBA& border_color,
            const mono::Color::RGBA& text_color,
            const mono::Color::RGBA& subtext_color);
        void Update(const mono::UpdateContext& update_context) override;

        void SetText(const char* text);
        void SetSubText(const char* sub_text);
        void SetTextColor(const mono::Color::RGBA& color);
        void SetSubTextColor(const mono::Color::RGBA& color);
        void SetAlpha(float alpha);

    private:

        class UISquareElement* m_background;
        class UITextElement* m_main_text_element;
        class UITextElement* m_sub_text_element;
    };
}
