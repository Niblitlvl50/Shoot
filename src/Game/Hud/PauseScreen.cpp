
#include "PauseScreen.h"

#include "Rendering/RenderSystem.h"
#include "Rendering/Text/TextFunctions.h"
#include "FontIds.h"

#include "Input/InputSystem.h"
#include "UI/UISystem.h"
#include "GameCamera/CameraSystem.h"
#include "Camera/ICamera.h"

#include "EntitySystem/IEntityManager.h"
#include "TransformSystem/TransformSystem.h"

#include "EventHandler/EventHandler.h"
#include "Events/QuitEvent.h"
#include "Events/PauseEvent.h"

using namespace game;

PauseScreen::PauseScreen(
    mono::TransformSystem* transform_system,
    mono::InputSystem* input_system,
    mono::IEntityManager* entity_manager,
    mono::EventHandler* event_handler,
    game::UISystem* ui_system)
    : m_transform_system(transform_system)
    , m_input_system(input_system)
    , m_ui_system(ui_system)
    , m_quit_proxy(ui_system, transform_system, entity_manager)
    , m_close_proxy(ui_system, transform_system, entity_manager)
{
    const float background_width = 10.0f;
    const float background_height = 5.5f;

    const float background_x = -(background_width / 2.0f);
    const float background_y = -(background_height / 2.0f);

    const FontId font_id = FontId::RUSSOONE_TINY;

    UISquareElement* background_element = new UISquareElement(background_width, background_height, mono::Color::BLACK, mono::Color::GRAY, 1.0f);
    background_element->SetPosition(background_x, background_y);

    UITextElement* pause_text =
        new UITextElement(font_id, "Pause", mono::FontCentering::HORIZONTAL, mono::Color::GRAY);
    pause_text->SetPosition(0.0f, background_y + background_height - 0.4f);

    m_quit_text =
        new UITextElement(font_id, "Quit", mono::FontCentering::DEFAULT_CENTER, mono::Color::GRAY);
    m_quit_text->SetPosition(background_x + 0.2f, background_y + 0.15f);

    const char* close_text = "Close";
    const mono::TextMeasurement close_text_measurement = mono::MeasureString(font_id, close_text);

    m_close_text =
        new UITextElement(font_id, close_text, mono::FontCentering::DEFAULT_CENTER, mono::Color::GRAY);
    m_close_text->SetPosition(background_x + background_width - close_text_measurement.size.x - 0.2f, background_y + 0.15f);

    m_input_layout = new UITextureElement();
    m_input_layout->SetScale(0.0075f);

    const UIItemCallback close_callback = [event_handler](uint32_t entity_id) {
        event_handler->DispatchEvent(event::PauseEvent(false));
    };
    m_quit_proxy.SetItemCallback(close_callback);
    m_close_proxy.SetItemCallback(close_callback);

    AddChild(background_element);
    AddChild(pause_text);
    AddChild(m_quit_text);
    AddChild(m_close_text);
    AddChild(m_input_layout);
}

void PauseScreen::ShowAt(const math::Vector& position)
{
    m_position = position;

    const math::Matrix& root_transform = Transform();

    const UINavigationSetup quit_nav_setup = {
        mono::INVALID_ID, m_close_proxy.GetEntityId(), mono::INVALID_ID, mono::INVALID_ID
    };
    m_quit_proxy.UpdateUIItem(
        root_transform * m_quit_text->Transform(), m_quit_text->GetBounds(), "level_aborted", quit_nav_setup);

    const UINavigationSetup close_nav_setup = {
        m_quit_proxy.GetEntityId(), mono::INVALID_ID, mono::INVALID_ID, mono::INVALID_ID
    };
    m_close_proxy.UpdateUIItem(
        root_transform * m_close_text->Transform(), m_close_text->GetBounds(), "close", close_nav_setup);

    const mono::InputContextType most_recent_input = m_input_system->GetMostRecentGlobalInput();
    const char* input_layout_texture =
        (most_recent_input == mono::InputContextType::Controller) ? 
            "res/textures/gamepad/gamepad_button_layout.png" : "res/textures/gamepad/keyboard_layout.png";
    m_input_layout->SetTexture(input_layout_texture);

    Show();
}

void PauseScreen::Show()
{
    UIElement::Show();
    m_ui_system->Enable();
}

void PauseScreen::Hide()
{
    UIElement::Hide();
    m_ui_system->Disable();
}
