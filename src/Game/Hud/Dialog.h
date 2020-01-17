
#pragma once

#include <Zone/EntityBase.h>

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
            float icon_scale = 1.0f;
        };

        UIDialog(
            const std::string& description,
            const std::vector<Option>& options,
            const mono::Color::RGBA& background_color,
            const mono::Color::RGBA& text_color);

    private:

        void Update(const mono::UpdateContext& update_context);
        void Draw(mono::IRenderer& renderer) const;

        const std::vector<Option> m_options;
    };
}
