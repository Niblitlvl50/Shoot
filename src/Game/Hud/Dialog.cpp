
#include "Dialog.h"
#include "UIElements.h"
#include "FontIds.h"

#include <Rendering/Text/TextFunctions.h>

using namespace game;

namespace
{
    float MeasureOptionsHeight(int font_id, const std::vector<UIDialog::Option>& options)
    {
        float height = 0.0f;

        for(const UIDialog::Option& option : options)
            height += mono::MeasureString(font_id, option.text.c_str()).y;

        return height;
    }
}

UIDialog::UIDialog(
    const std::string& description,
    const std::vector<Option>& options,
    const mono::Color::RGBA& background_color,
    const mono::Color::RGBA& text_color)
{
    constexpr int font_id = FontId::PIXELETTE_SMALL;
    constexpr math::Vector padding(0.5f, 0.5f);
    constexpr float option_spacing = 0.3f;

    const math::Vector description_text_size = mono::MeasureString(font_id, description.c_str());
    const float options_height = MeasureOptionsHeight(font_id, options);
    const float options_spacing_total = options.size() * option_spacing;

    const math::Vector bottom_left = -(math::Vector(0.0f, options_height + options_spacing_total) + padding + math::Vector(0.0f, padding.y));
    const math::Vector top_right = description_text_size + padding;
    const math::Quad dialog_size(bottom_left, top_right);

    AddChild(new UISquareElement(dialog_size, background_color, text_color, 0.1f));
    AddChild(new UITextElement(font_id, description, false, text_color));

    float y_shift = description_text_size.y + padding.y;

    for(const Option& option : options)
    {
        const math::Vector option_text_size = mono::MeasureString(font_id, option.text.c_str());

        const std::vector<std::string>& option_sprites = {
            option.icon
        };

        auto option_text = new UITextElement(font_id, option.text, false, text_color);
        auto option_icon = new UISpriteElement(option_sprites);

        option_text->SetPosition(math::Vector(2.0f, -y_shift));
        option_icon->SetPosition(math::Vector(1.0f, -y_shift + 0.25f));

        option_icon->SetScale(math::Vector(option.icon_scale, option.icon_scale));
        
        AddChild(option_text);
        AddChild(option_icon);

        y_shift += option_text_size.y + option_spacing;
    }
}

void UIDialog::Update(const mono::UpdateContext& update_context)
{}

void UIDialog::Draw(mono::IRenderer& renderer) const
{}

