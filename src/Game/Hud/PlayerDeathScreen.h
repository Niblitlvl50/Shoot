
#pragma once

#include "MonoFwd.h"
#include "Math/Vector.h"
#include "System/System.h"

#include "UIElements.h"

namespace game
{
    struct PlayerInfo;

    class PlayerDeathScreen : public game::UIOverlay
    {
    public:

        PlayerDeathScreen(const PlayerInfo& player_info, mono::EventHandler* event_handler);
        void Update(const mono::UpdateContext& update_context) override;

    private:

        const PlayerInfo& m_player_info;
        math::Vector m_screen_position;
        math::Vector m_offscreen_position;
        mono::EventHandler* m_event_handler;

        float m_timer;
        System::ControllerState m_last_state;
    };

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
        void SetAlpha(float alpha);

        class UISquareElement* m_background;
        class UITextElement* m_main_text_element;
        class UITextElement* m_sub_text_element;
    };
}
