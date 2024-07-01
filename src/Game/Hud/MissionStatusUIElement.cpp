
#include "MissionStatusUIElement.h"
#include "FontIds.h"

using namespace game;

MissionStatusUIElement::MissionStatusUIElement(float width, float height, const mono::Color::RGBA& background_color)
    : UISquareElement(width, height, background_color)
{
    m_mission_name_text = new UITextElement(FontId::MITR_SMALL, "", mono::FontCentering::DEFAULT_CENTER, mono::Color::OFF_WHITE);
    m_mission_name_text->SetPosition(0.5f, 1.0f);

    m_mission_description_text = new UITextElement(FontId::MITR_TINY, "", mono::FontCentering::DEFAULT_CENTER, mono::Color::GRAY);
    m_mission_description_text->SetPosition(0.65f, 0.25f);

    m_icon = new UISpriteElement("res/sprites/ui_icon_check.sprite");
    m_icon->SetPosition(width - 1.0f, height / 2.0f);
    m_icon->SetScale(4.0f);

    AddChild(m_mission_name_text);
    AddChild(m_mission_description_text);
    AddChild(m_icon);
}

void MissionStatusUIElement::SetText(const std::string& text)
{
    m_mission_name_text->SetText(text);
}

void MissionStatusUIElement::SetDescription(const std::string& description)
{
    m_mission_description_text->SetText(description);
}

void MissionStatusUIElement::ShowIcon(bool show)
{
    if(show)
        m_icon->Show();
    else
        m_icon->Hide();
}

