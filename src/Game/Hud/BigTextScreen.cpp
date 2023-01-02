
#include "BigTextScreen.h"

#include "FontIds.h"
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
{
    const bool hide_background = ((component_flags & Components::BACKGROUND) == 0);
    const bool hide_text = ((component_flags & Components::TEXT) == 0);
    const bool hide_subtext = ((component_flags & Components::SUBTEXT) == 0);

    const float half_width = m_width / 2.0f;
    const float half_height = m_height / 2.0f;
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
    mono::Color::RGBA color = m_background->GetColor();
    color.alpha = alpha;

    mono::Color::RGBA border_color = m_background->GetBorderColor();
    border_color.alpha = alpha;

    m_background->SetColor(color);
    m_background->SetBorderColor(border_color);

    m_main_text_element->SetAlpha(alpha);
    m_sub_text_element->SetAlpha(alpha);
}
