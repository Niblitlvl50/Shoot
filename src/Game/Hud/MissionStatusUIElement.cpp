
#include "MissionStatusUIElement.h"
#include "FontIds.h"

using namespace game;

MissionStatusUIElement::MissionStatusUIElement(float width, float height, const mono::Color::RGBA& background_color)
    : UISquareElement(width, height, background_color)
{
    m_mission_name_text = new UITextElement(FontId::MITR_SMALL, "", mono::FontCentering::DEFAULT_CENTER, mono::Color::GOLDEN_YELLOW);
    m_mission_name_text->SetPosition(0.5f, 1.5f);

    m_mission_description_text = new UITextElement(FontId::MITR_TINY, "", mono::FontCentering::DEFAULT_CENTER, mono::Color::LIGHT_GRAY);
    m_mission_description_text->SetPosition(0.75f, 0.5f);

    AddChild(m_mission_name_text);
    AddChild(m_mission_description_text);
}

void MissionStatusUIElement::SetText(const std::string& text)
{
    m_mission_name_text->SetText(text);
}

void MissionStatusUIElement::SetDescription(const std::string& description)
{
    m_mission_description_text->SetText(description);
}

