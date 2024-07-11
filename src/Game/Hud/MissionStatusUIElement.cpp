
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

void MissionStatusUIElement::ShowIcon(bool show, int index)
{
    m_icon->SetActiveSprite(index, 0);
    if(show)
        m_icon->Show();
    else
        m_icon->Hide();
}

