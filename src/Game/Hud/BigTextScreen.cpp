
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
    const mono::Color::RGBA& subtext_color)
    : UIOverlay(50.0f, 50.0f / mono::GetWindowAspect())
{
    const float half_width = m_width / 2.0f;
    const float half_height = m_height / 2.0f;
    const float square_height = m_height * 0.3f;

    m_background = new UISquareElement(m_width, square_height, color, border_color, 1.0f);
    m_background->SetPosition(math::Vector(0.0f, half_height - (square_height / 2.0f)));

    m_main_text_element =
        new UITextElement(FontId::RUSSOONE_MEGA, text, mono::FontCentering::HORIZONTAL_VERTICAL, text_color);
    m_main_text_element->SetPosition(math::Vector(half_width, half_height + 1.5f));

    m_sub_text_element =
        new UITextElement(FontId::RUSSOONE_LARGE, sub_text, mono::FontCentering::HORIZONTAL_VERTICAL, subtext_color);
    m_sub_text_element->SetPosition(math::Vector(half_width, half_height - 1.0f));

    AddChild(m_background);
    AddChild(m_main_text_element);
    AddChild(m_sub_text_element);
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
