
#include "PauseScreen.h"

#include "Rendering/RenderSystem.h"
#include "Rendering/Text/TextFunctions.h"
#include "FontIds.h"

using namespace game;

PauseScreen::PauseScreen()
    : game::UIOverlay(1200, 1200.0f / mono::GetWindowAspect())
{
    const float background_width = 1000.0f;
    const float background_height = 600.0f;

    const float background_x = (m_width - background_width) / 2.0f;
    const float background_y = (m_height - background_height) / 2.0f;

    UISquareElement* background_element = new UISquareElement(background_width, background_height, mono::Color::BLACK, mono::Color::GRAY, 1.0f);
    background_element->SetPosition(background_x, background_y);

    UITextElement* pause_text =
        new UITextElement(FontId::RUSSOONE_MEDIUM, "Pause", mono::FontCentering::DEFAULT_CENTER, mono::Color::OFF_WHITE);
    pause_text->SetPosition(background_x + 50.0f, background_y + background_height - 50.0f);

    const math::Vector text_size = mono::MeasureString(FontId::RUSSOONE_MEDIUM, "start to exit");

    UITextElement* exit_text =
        new UITextElement(FontId::RUSSOONE_MEDIUM, "start to exit", mono::FontCentering::DEFAULT_CENTER, mono::Color::GRAY);
    exit_text->SetPosition(background_x + background_width - text_size.x - 50.0f, background_y + 30.0f);

    UITextureElement* texture_element = new UITextureElement("res/textures/gamepad/gamepad_button_layout.png");
    texture_element->SetScale(0.75f);
    texture_element->SetPosition(m_width / 2.0f, m_height / 2.0f);

    // background_element->AddChild(pause_text);
    // background_element->AddChild(exit_text);
    // background_element->AddChild(texture_element);

    AddChild(background_element);
    AddChild(pause_text);
    AddChild(exit_text);
    AddChild(texture_element);
}
