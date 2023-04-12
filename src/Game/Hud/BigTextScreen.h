
#pragma once

#include "MonoFwd.h"
#include "Math/Vector.h"
#include "Rendering/RenderFwd.h"
#include "UIElements.h"

#define ENUM_BIT(n) (1 << (n))

namespace game
{
    class BigTextScreen : public game::UIOverlay
    {
    public:

        enum Components
        {
            BACKGROUND  = ENUM_BIT(0),
            TEXT        = ENUM_BIT(1),
            SUBTEXT     = ENUM_BIT(2),
            ALL         = (~(uint32_t)0),
        };

        BigTextScreen(
            const char* main_text,
            const char* sub_text,
            const mono::Color::RGBA& color,
            const mono::Color::RGBA& border_color,
            const mono::Color::RGBA& text_color,
            const mono::Color::RGBA& subtext_color,
            uint32_t component_flags = Components::ALL);
        void Update(const mono::UpdateContext& update_context) override;

        void SetText(const char* text);
        void SetSubText(const char* sub_text);
        void SetTextColor(const mono::Color::RGBA& color);
        void SetSubTextColor(const mono::Color::RGBA& color);
        void SetAlpha(float alpha);

        enum class FadeState
        {
            FADE_IN,
            SHOWN,
            FADE_OUT
        };
        struct FadePattern
        {
            FadeState fade_state;
            float duration;
        };
        using Callback = std::function<void ()>;
        void ShowWithFadePattern(const std::vector<FadePattern>& fade_pattern, const Callback& callback);

    private:

        class UISquareElement* m_background;
        class UITextElement* m_main_text_element;
        class UITextElement* m_sub_text_element;

        float m_fade_timer;
        uint32_t m_fade_index;
        std::vector<FadePattern> m_fade_pattern;
        Callback m_fade_pattern_callback;
    };
}
