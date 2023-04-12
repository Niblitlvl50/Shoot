
#include "BigTextScreen.h"

#include "FontIds.h"
#include "Math/EasingFunctions.h"
#include "Rendering/RenderSystem.h"
#include "System/System.h"

using namespace game;

BigTextScreen::BigTextScreen(
    const char* text,
    const char* sub_text,
    const mono::Color::RGBA& color,
    const mono::Color::RGBA& border_color,
    const mono::Color::RGBA& text_color,
    const mono::Color::RGBA& subtext_color,
    uint32_t component_flags)
    : UIOverlay(50.0f, 50.0f / mono::RenderSystem::GetWindowAspect())
    , m_fade_timer(0.0f)
{
    const bool hide_background = ((component_flags & Components::BACKGROUND) == 0);
    const bool hide_text = ((component_flags & Components::TEXT) == 0);
    const bool hide_subtext = ((component_flags & Components::SUBTEXT) == 0);

    const float half_width = m_width / 2.0f;
    const float half_height = m_height * 0.65f;
    const float square_height = m_height * 0.3f;

    m_background = new UISquareElement(m_width, square_height, color, border_color, 1.0f);
    m_background->SetPosition(math::Vector(0.0f, half_height - (square_height / 2.0f)));

    const float text_offset = hide_subtext ? 0.0f : 1.5f;
    m_main_text_element =
        new UITextElement(FontId::RUSSOONE_MEGA, text, mono::FontCentering::HORIZONTAL_VERTICAL, text_color);
    m_main_text_element->SetPosition(math::Vector(half_width, half_height + text_offset));

    const float sub_text_offset = hide_text ? 0.0f : 1.0f;
    m_sub_text_element =
        new UITextElement(FontId::RUSSOONE_LARGE, sub_text, mono::FontCentering::HORIZONTAL_VERTICAL, subtext_color);
    m_sub_text_element->SetPosition(math::Vector(half_width, half_height - sub_text_offset));

    AddChild(m_background);
    AddChild(m_main_text_element);
    AddChild(m_sub_text_element);

    if(hide_background)
        m_background->Hide();
    if(hide_text)
        m_main_text_element->Hide();
    if(hide_subtext)
        m_sub_text_element->Hide();
}

void BigTextScreen::Update(const mono::UpdateContext& update_context)
{
    UIOverlay::Update(update_context);

    // if has active fade pattern, apply that...
    if(!m_fade_pattern.empty())
    {
        m_fade_timer += update_context.delta_s;

        const FadePattern& fade_pattern = m_fade_pattern[m_fade_index];
        
        float alpha_value = 0.0f;

        switch(fade_pattern.fade_state)
        {
        case FadeState::FADE_IN:
            alpha_value = math::EaseInCubic(m_fade_timer, fade_pattern.duration, 0.0f, 1.0f);
            break;
        case FadeState::FADE_OUT:
            alpha_value = math::EaseInCubic(m_fade_timer, fade_pattern.duration, 1.0f, -1.0f);
            break;
        case FadeState::SHOWN:
            alpha_value = 1.0f;
            // Do nothing...
            break;
        }

        SetAlpha(alpha_value);

        if(m_fade_timer > fade_pattern.duration)
        {
            m_fade_timer = 0.0f;
            m_fade_index++;
        }

        if(m_fade_index == m_fade_pattern.size())
        {
            m_fade_pattern.clear();

            if(m_fade_pattern_callback)
                m_fade_pattern_callback();
        }
    }
}

void BigTextScreen::SetText(const char* text)
{
    m_main_text_element->SetText(text);
}

void BigTextScreen::SetSubText(const char* sub_text)
{
    m_sub_text_element->SetText(sub_text);
}

void BigTextScreen::SetTextColor(const mono::Color::RGBA& color)
{
    m_main_text_element->SetColor(color);
}

void BigTextScreen::SetSubTextColor(const mono::Color::RGBA& color)
{
    m_sub_text_element->SetColor(color);
}

void BigTextScreen::SetAlpha(float alpha)
{
    const mono::Color::RGBA color = mono::Color::MakeWithAlpha(m_background->GetColor(), alpha);
    const mono::Color::RGBA border_color = mono::Color::MakeWithAlpha(m_background->GetBorderColor(), alpha);

    m_background->SetColor(color);
    m_background->SetBorderColor(border_color);

    m_main_text_element->SetAlpha(alpha);
    m_sub_text_element->SetAlpha(alpha);
}

void BigTextScreen::ShowWithFadePattern(const std::vector<FadePattern>& fade_pattern, const Callback& callback)
{
    m_fade_timer = 0.0f;
    m_fade_index = 0;
    m_fade_pattern = fade_pattern;
    m_fade_pattern_callback = callback;

    UIOverlay::Show();
}
