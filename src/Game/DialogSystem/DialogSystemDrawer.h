
#pragma once

#include "MonoFwd.h"
#include "Hud/UIElements.h"
#include "DialogSystem.h"

#include <cstdint>

namespace game
{
    class DialogSystemDrawer : public game::UIOverlay
    {
    public:

        DialogSystemDrawer(const class DialogSystem* dialog_system);
        void Update(const mono::UpdateContext& context) override;

    private:

        void SetMessage(const struct DialogLine& line);
        void SetSelectedOption(const struct DialogLine& line);
        void SetTextAlpha(float alpha);

        const DialogSystem* m_dialog_system;
        uint32_t m_current_line_id;
        int m_shown_selection;

        UIElement* m_panel;
        UISquareElement* m_background;
        UISpriteElement* m_portrait;
        UITextElement* m_speaker_text;
        UITextElement* m_message_lines[3];
        UITextElement* m_option_texts[MAX_DIALOG_OPTIONS];
    };
}
