
#include "PauseScreen.h"

#include "Rendering/RenderSystem.h"
#include "FontIds.h"

using namespace game;

PauseScreen::PauseScreen()
    : game::UIOverlay(1200, 1200.0f / mono::GetWindowAspect())
{
    const float background_width = 1000.0f;
    const float background_height = 600.0f;

    const float background_x = (m_width - background_width) / 2.0f;
    const float background_y = (m_height - background_height) / 2.0f;

    UISquareElement* background_element = new UISquareElement(background_width, background_height, mono::Color::BLACK);
    background_element->SetPosition(background_x, background_y);

    UITextElement* pause_text =
        new UITextElement(shared::FontId::RUSSOONE_MEDIUM, "Pause", mono::FontCentering::DEFAULT_CENTER, mono::Color::OFF_WHITE);
    pause_text->SetPosition(50.0f, background_height - 50.0f);

    UITextElement* exit_text =
        new UITextElement(shared::FontId::RUSSOONE_MEDIUM, "start to exit", mono::FontCentering::DEFAULT_CENTER, mono::Color::GRAY);
    exit_text->SetPosition(50.0f, 0.0f);

    UITextureElement* texture_element = new UITextureElement("res/textures/gamepad/gamepad_button_layout.png");
    texture_element->SetScale(0.75f);
    texture_element->SetPosition(background_width / 2.0f, background_height / 2.0f);

    // background_element->AddChild(pause_text);
    // background_element->AddChild(exit_text);
    // background_element->AddChild(texture_element);

    AddChild(background_element);
    AddChild(pause_text);
    AddChild(exit_text);
    AddChild(texture_element);
}
