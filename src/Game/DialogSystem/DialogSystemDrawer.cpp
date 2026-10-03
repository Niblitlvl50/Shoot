
#include "DialogSystemDrawer.h"
#include "DialogSystem.h"
#include "FontIds.h"

#include "Math/EasingFunctions.h"
#include "Rendering/RenderSystem.h"
#include "Rendering/Color.h"
#include "Rendering/Text/TextFunctions.h"

#include <algorithm>
#include <limits>
#include <sstream>
#include <vector>

namespace tweak_values
{
    constexpr float panel_width = 36.0f;
    constexpr float panel_min_height = 4.5f;
    constexpr float panel_bottom_y = 6.5f; // Above the player hud, the panel grows upwards with more options.
    constexpr float text_margin = 1.0f;
    constexpr float top_margin = 0.2f;
    constexpr float bottom_margin = 0.4f;
    constexpr float line_spacing = 0.75f;
    constexpr float option_spacing = 0.7f;
    constexpr float options_gap = 0.3f;
    constexpr float portrait_margin = 0.4f;
    constexpr float portrait_size = panel_min_height - (portrait_margin * 2.0f);
    constexpr float open_time_s = 0.25f;
}

namespace
{
    constexpr uint32_t NO_LINE = std::numeric_limits<uint32_t>::max();
    constexpr int NO_SELECTION = -1;
    constexpr game::FontId font_id = game::FontId::RUSSOONE_TINY;

    std::vector<std::string> WrapText(const std::string& text, float max_width)
    {
        std::vector<std::string> lines;
        std::string current_line;

        std::istringstream stream(text);
        std::string word;
        while(stream >> word)
        {
            const std::string candidate = current_line.empty() ? word : current_line + " " + word;
            const mono::TextMeasurement measurement = mono::MeasureString(font_id, candidate.c_str());
            if(measurement.size.x > max_width && !current_line.empty())
            {
                lines.push_back(current_line);
                current_line = word;
            }
            else
            {
                current_line = candidate;
            }
        }

        if(!current_line.empty())
            lines.push_back(current_line);

        return lines;
    }
}

using namespace game;

DialogSystemDrawer::DialogSystemDrawer(const DialogSystem* dialog_system)
    : UIOverlay(50.0f, 50.0f / mono::RenderSystem::GetWindowAspect())
    , m_dialog_system(dialog_system)
    , m_current_line_id(NO_LINE)
    , m_shown_selection(NO_SELECTION)
{
    // The panel's origin is its bottom center, so opening scales it up from the bottom edge.
    m_panel = new UIElement();
    m_panel->SetPosition(m_width / 2.0f, tweak_values::panel_bottom_y);
    m_panel->Hide();

    m_background = new UISquareElement(
        tweak_values::panel_width, tweak_values::panel_min_height, mono::Color::RGBA(0.0f, 0.0f, 0.0f, 0.85f), mono::Color::GRAY, 1.0f);
    m_panel->AddChild(m_background);

    m_portrait = new UISpriteElement();
    m_panel->AddChild(m_portrait);

    m_speaker_text = new UITextElement(font_id, "", mono::Color::GOLDEN_YELLOW);
    m_speaker_text->SetAchorPoint(mono::AnchorPoint::BOTTOM_LEFT);
    m_panel->AddChild(m_speaker_text);

    for(UITextElement*& line : m_message_lines)
    {
        line = new UITextElement(font_id, "", mono::Color::OFF_WHITE);
        line->SetAchorPoint(mono::AnchorPoint::BOTTOM_LEFT);
        m_panel->AddChild(line);
    }

    for(UITextElement*& option : m_option_texts)
    {
        option = new UITextElement(font_id, "", mono::Color::GRAY);
        option->SetAchorPoint(mono::AnchorPoint::BOTTOM_LEFT);
        m_panel->AddChild(option);
    }

    AddChild(m_panel);
}

void DialogSystemDrawer::Update(const mono::UpdateContext& context)
{
    UIOverlay::Update(context);

    const DialogLine* active_line = m_dialog_system->GetActiveDialog();
    if(!active_line)
    {
        m_panel->Hide();
        m_current_line_id = NO_LINE;
        return;
    }

    if(active_line->id != m_current_line_id)
    {
        SetMessage(*active_line);
        m_panel->Show();
        m_current_line_id = active_line->id;
        m_shown_selection = NO_SELECTION;
    }

    if(!active_line->data.options.empty() && active_line->selected_option != m_shown_selection)
    {
        SetSelectedOption(*active_line);
        m_shown_selection = active_line->selected_option;
    }

    // Opens and closes vertically, so the panel never slides over the player hud.
    const float elapsed_s = m_dialog_system->GetActiveDialogTime();
    const float open_in = elapsed_s / tweak_values::open_time_s;
    const float open_out = (active_line->data.duration - elapsed_s) / tweak_values::open_time_s;
    const float open = std::clamp(std::min(open_in, open_out), 0.0f, 1.0f);
    const float eased_open = math::EaseInOutCubic(open, 1.0f, 0.0f, 1.0f);

    m_panel->SetScale(math::Vector(1.0f, eased_open));
    SetTextAlpha(eased_open);
}

void DialogSystemDrawer::SetMessage(const DialogLine& dialog_line)
{
    const DialogData& line = dialog_line.data;

    const bool has_portrait = !line.sprite_file.empty();
    const float panel_left = -(tweak_values::panel_width / 2.0f);
    const float portrait_offset = has_portrait ? (tweak_values::portrait_size + tweak_values::portrait_margin) : 0.0f;
    const float text_x = panel_left + portrait_offset + tweak_values::text_margin;

    const float max_line_width = tweak_values::panel_width - portrait_offset - (tweak_values::text_margin * 2.0f);
    const std::vector<std::string> wrapped_lines = WrapText(line.message, max_line_width);

    const size_t n_message_lines = std::min(wrapped_lines.size(), std::size(m_message_lines));
    const size_t n_options = std::min(line.options.size(), std::size(m_option_texts));

    float content_height = tweak_values::top_margin + tweak_values::bottom_margin;
    content_height += tweak_values::line_spacing * (1 + n_message_lines);
    if(n_options > 0)
        content_height += tweak_values::options_gap + (tweak_values::option_spacing * n_options);

    const float panel_height = std::max(tweak_values::panel_min_height, content_height);
    m_background->SetPosition(0.0f, panel_height / 2.0f);
    m_background->SetScale(math::Vector(1.0f, panel_height / tweak_values::panel_min_height));

    if(has_portrait)
    {
        m_portrait->SetSprite(line.sprite_file);

        const math::Vector sprite_size = math::Size(m_portrait->LocalBoundingBox());
        const float sprite_max_size = std::max(sprite_size.x, sprite_size.y);
        const float portrait_scale = (sprite_max_size > 0.0f) ? (tweak_values::portrait_size / sprite_max_size) : 1.0f;
        m_portrait->SetScale(portrait_scale);

        const float portrait_x = panel_left + tweak_values::portrait_margin + (tweak_values::portrait_size / 2.0f);
        const float portrait_y = panel_height - tweak_values::portrait_margin - (tweak_values::portrait_size / 2.0f);
        m_portrait->SetPosition(portrait_x, portrait_y);
        m_portrait->Show();
    }
    else
    {
        m_portrait->Hide();
    }

    // Rows are laid out from the top of the panel down.
    float row_y = panel_height - tweak_values::top_margin;

    row_y -= tweak_values::line_spacing;
    m_speaker_text->SetText(line.speaker);
    m_speaker_text->SetPosition(text_x, row_y);

    for(size_t index = 0; index < std::size(m_message_lines); ++index)
    {
        const bool used = (index < n_message_lines);
        if(used)
            row_y -= tweak_values::line_spacing;

        m_message_lines[index]->SetText(used ? wrapped_lines[index] : std::string());
        m_message_lines[index]->SetPosition(text_x, row_y);
    }

    if(n_options > 0)
        row_y -= tweak_values::options_gap;

    for(size_t index = 0; index < std::size(m_option_texts); ++index)
    {
        UITextElement* option = m_option_texts[index];
        if(index < n_options)
        {
            row_y -= tweak_values::option_spacing;
            option->SetPosition(text_x, row_y);
            option->Show();
        }
        else
        {
            option->Hide();
        }
    }
}

void DialogSystemDrawer::SetSelectedOption(const DialogLine& dialog_line)
{
    const size_t n_options = std::min(dialog_line.data.options.size(), std::size(m_option_texts));
    for(size_t index = 0; index < n_options; ++index)
    {
        const std::string& option_text = dialog_line.data.options[index].text;
        const bool selected = (static_cast<int>(index) == dialog_line.selected_option);

        UITextElement* option = m_option_texts[index];
        option->SetText(selected ? ("> " + option_text) : ("   " + option_text));
        option->SetColor(selected ? mono::Color::GOLDEN_YELLOW : mono::Color::GRAY);
    }
}

void DialogSystemDrawer::SetTextAlpha(float alpha)
{
    m_speaker_text->SetAlpha(alpha);
    for(UITextElement* line : m_message_lines)
        line->SetAlpha(alpha);
    for(UITextElement* option : m_option_texts)
        option->SetAlpha(alpha);
}
