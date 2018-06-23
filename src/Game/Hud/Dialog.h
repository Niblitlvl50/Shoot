
#pragma once

#include <Entity/EntityBase.h>

#include <string>

namespace game
{
    class UIDialog : public mono::EntityBase
    {
    public:

        struct Option
        {
            std::string text;
            std::string icon;
        };

        UIDialog(
            const std::string& description,
            const std::vector<Option>& options,
            const mono::Color::RGBA& background_color,
            const mono::Color::RGBA& text_color);

    private:

        void Update(unsigned int delta);
        void Draw(mono::IRenderer& renderer) const;

        const std::vector<Option> m_options;
    };
}
