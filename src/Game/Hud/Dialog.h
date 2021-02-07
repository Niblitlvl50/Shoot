
#pragma once

#include "UIElements.h"
#include "Rendering/Color.h"

#include <string>
#include <vector>

namespace game
{
    class UIDialog : public UIOverlay
    {
    public:

        struct Option
        {
            std::string text;
            std::string icon;
            float icon_scale = 1.0f;
        };

        UIDialog(
            const std::string& description,
            const std::vector<Option>& options,
            const mono::Color::RGBA& background_color,
            const mono::Color::RGBA& text_color);

    private:

        const std::vector<Option> m_options;
    };
}
