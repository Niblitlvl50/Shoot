
#include "PauseScreen.h"

#include "Rendering/RenderSystem.h"
#include "Rendering/Text/TextFunctions.h"
#include "FontIds.h"

using namespace game;

PauseScreen::PauseScreen()
    : game::UIOverlay(200, 200.0f / mono::GetWindowAspect())
{
    const float background_width = 160.0f;
    const float background_height = 90.0f;

    const float background_x = (m_width - background_width) / 2.0f;
    const float background_y = (m_height - background_height) / 2.0f;

    const FontId font_id = FontId::RUSSOONE_MEGA;

    UISquareElement* background_element = new UISquareElement(background_width, background_height, mono::Color::BLACK, mono::Color::GRAY, 1.0f);
    background_element->SetPosition(background_x, background_y);

    UITextElement* pause_text =
        new UITextElement(font_id, "Pause", mono::FontCentering::DEFAULT_CENTER, mono::Color::GRAY);
    pause_text->SetPosition(background_x + 2.0f, background_y + background_height - 5.0f);

    const char* start_to_exit = "start to exit";
    const math::Vector text_size = mono::MeasureString(font_id, start_to_exit);

    UITextElement* exit_text =
        new UITextElement(font_id, start_to_exit, mono::FontCentering::DEFAULT_CENTER, mono::Color::GRAY);
    exit_text->SetPosition(background_x + background_width - text_size.x - 2.0f, background_y + 2.5f);

    UITextureElement* texture_element = new UITextureElement("res/textures/gamepad/gamepad_button_layout.png");
    texture_element->SetPosition(m_width / 2.0f, m_height / 2.0f);
    texture_element->SetScale(0.125f);

    AddChild(background_element);
    AddChild(pause_text);
    AddChild(exit_text);
    AddChild(texture_element);
}
