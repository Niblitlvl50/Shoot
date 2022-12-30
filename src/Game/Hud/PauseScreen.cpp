
#include "PauseScreen.h"

#include "Rendering/RenderSystem.h"
#include "Rendering/Text/TextFunctions.h"
#include "FontIds.h"

#include "Input/InputSystem.h"
#include "UI/UISystem.h"
#include "GameCamera/CameraSystem.h"
#include "Camera/ICamera.h"

#include "EntitySystem/IEntityManager.h"

using namespace game;

PauseScreen::PauseScreen(
    mono::InputSystem* input_system,
    mono::IEntityManager* entity_manager,
    game::UISystem* ui_system,
    game::CameraSystem* camera_system)
    //: game::UIOverlay(200, 200.0f / mono::GetWindowAspect())
    : m_input_system(input_system)
    , m_entity_manager(entity_manager)
    , m_ui_system(ui_system)
    , m_camera_system(camera_system)
{
    const float background_width = 10.0f;
    const float background_height = 5.5f;

    const float background_x = -(background_width / 2.0f);
    const float background_y = -(background_height / 2.0f);

    const FontId font_id = FontId::RUSSOONE_TINY;

    UISquareElement* background_element = new UISquareElement(background_width, background_height, mono::Color::BLACK, mono::Color::GRAY, 1.0f);
    background_element->SetPosition(background_x, background_y);

    UITextElement* pause_text =
        new UITextElement(font_id, "Pause", mono::FontCentering::DEFAULT_CENTER, mono::Color::GRAY);
    pause_text->SetPosition(background_x + 0.2f, background_y + background_height - 0.4f);

    const char* start_to_exit = "start to exit";
    const math::Vector text_size = mono::MeasureString(font_id, start_to_exit);

    UITextElement* exit_text =
        new UITextElement(font_id, start_to_exit, mono::FontCentering::DEFAULT_CENTER, mono::Color::GRAY);
    exit_text->SetPosition(background_x + background_width - text_size.x - 0.2f, background_y + 0.15f);

    UITextureElement* texture_element = new UITextureElement("res/textures/gamepad/gamepad_button_layout.png");
    texture_element->SetScale(0.0075f);

    m_entity_manager->CreateEntity("ui_temp", { });

    AddChild(background_element);
    AddChild(pause_text);
    AddChild(exit_text);
    AddChild(texture_element);
}

void PauseScreen::Show()
{
    UIElement::Show();

    const mono::ICamera* camera = m_camera_system->GetActiveCamera();
    m_position = camera->GetPosition();
    m_ui_system->Enable();
}

void PauseScreen::Hide()
{
    UIElement::Hide();
    m_ui_system->Disable();
}
