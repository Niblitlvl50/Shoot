
#include "MissionStatusUIElement.h"
#include "FontIds.h"

using namespace game;

MissionStatusUIElement::MissionStatusUIElement(float width, float height, const mono::Color::RGBA& background_color)
    : UISquareElement(width, height, background_color)
{
    SetAchorPoint(mono::AnchorPoint::BOTTOM_LEFT);
    const math::Vector anchor_offset = GetAnchorOffset();

    m_mission_name_text = new UITextElement(FontId::MITR_SMALL, "", mono::Color::OFF_WHITE);
    m_mission_name_text->SetAchorPoint(mono::AnchorPoint::BOTTOM_LEFT);
    m_mission_name_text->SetPosition(-anchor_offset + math::Vector(0.25f, 1.15f));

    m_mission_description_text = new UITextElement(FontId::MITR_TINY, "", mono::Color::GRAY);
    m_mission_description_text->SetAchorPoint(mono::AnchorPoint::BOTTOM_LEFT);
    m_mission_description_text->SetPosition(-anchor_offset + math::Vector(0.5f, 0.35f));

    const std::vector<std::string> ui_sprites = {
        "res/sprites/ui_icon_check.sprite",
        "res/sprites/ui_icon_cross.sprite"
    };
    m_icon = new UISpriteElement(ui_sprites);
    m_icon->SetPosition(width / 2.0f - 1.0f, 0.0f);
    m_icon->SetScale(4.0f);

    m_timer_bar = new UIBarElement(width, 0.1f, mono::Color::DARK_GRAY, mono::Color::GOLDEN_YELLOW);
    m_timer_bar->SetPosition(-anchor_offset);
    m_timer_bar->Hide();

    m_timer_text = new UITextElement(FontId::MITR_TINY, "", mono::Color::OFF_WHITE);
    m_timer_text->SetPosition(width / 2.0f - 1.0f, 0.0f);
    m_timer_text->Hide();

    AddChild(m_mission_name_text);
    AddChild(m_mission_description_text);
    AddChild(m_icon);
    AddChild(m_timer_bar);
    AddChild(m_timer_text);
}

void MissionStatusUIElement::SetText(const std::string& text)
{
    m_mission_name_text->SetText(text);
}

void MissionStatusUIElement::SetDescription(const std::string& description)
{
    m_mission_description_text->SetText(description);
}

void MissionStatusUIElement::ShowIcon(bool show, int index)
{
    m_icon->SetActiveSprite(index, 0);
    if(show)
    {
        m_icon->Show();
        m_timer_text->Hide();
    }
    else
    {
        m_icon->Hide();
    }
}

void MissionStatusUIElement::ShowTimer(bool show)
{
    if(show)
    {
        m_timer_bar->Show();
        m_timer_text->Show();
    }
    else
    {
        m_timer_bar->Hide();
        m_timer_text->Hide();
    }
}

void MissionStatusUIElement::SetTime(float time_s, float max_time_s)
{
    const float fraction = math::Scale01Clamped(time_s, max_time_s, 0.0f);
    m_timer_bar->SetFraction(fraction);


    const int seconds = int(time_s) % 60;
    const int minutes = time_s / 60.0f;

    char text_buffer[1024] = {};
    std::snprintf(text_buffer, std::size(text_buffer), "%d:%.2d", minutes, seconds);

    m_timer_text->SetText(text_buffer);
}
