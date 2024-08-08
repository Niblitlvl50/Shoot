
#include "PauseScreen.h"

#include "Rendering/Text/TextFunctions.h"
#include "Rendering/RenderSystem.h"
#include "FontIds.h"

#include "Input/InputSystem.h"
#include "UI/UISystem.h"

#include "EntitySystem/Entity.h"
#include "TransformSystem/TransformSystem.h"

#include "EventHandler/EventHandler.h"
#include "Events/QuitEvent.h"
#include "Events/PauseEvent.h"
#include "System/Hash.h"

using namespace game;

PauseScreen::PauseScreen(
    const std::string& aborted_hash,
    mono::TransformSystem* transform_system,
    mono::InputSystem* input_system,
    mono::IEntityManager* entity_manager,
    mono::EventHandler* event_handler,
    game::UISystem* ui_system)
    : m_input_system(input_system)
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

    UITextElement* pause_text = new UITextElement(font_id, "Pause", mono::Color::GRAY);
    pause_text->SetPosition(0.0f, (background_height / 2.0f) - 0.25f);

    m_quit_text = new UITextElement(font_id, "Quit", mono::Color::GRAY);
    m_quit_text->SetAchorPoint(mono::AnchorPoint::BOTTOM_LEFT);
    m_quit_text->SetPosition(background_x + 0.2f, background_y + 0.15f);

    m_close_text = new UITextElement(font_id, "Close", mono::Color::GRAY);
    m_close_text->SetAchorPoint(mono::AnchorPoint::BOTTOM_RIGHT);
    m_close_text->SetPosition(background_x + background_width - 0.2f, background_y + 0.15f);

    m_input_layout = new UITextureElement();
    m_input_layout->SetScale(0.0075f);

    const UIItemCallback close_callback = [event_handler](uint32_t entity_id) {
        event_handler->DispatchEvent(event::PauseEvent(false));
    };
    m_quit_proxy.SetItemCallback(close_callback);
    m_close_proxy.SetItemCallback(close_callback);

    m_quit_proxy.SetItemState(UIItemState::Disabled);
    m_close_proxy.SetItemState(UIItemState::Disabled);

    AddChild(background_element);
    AddChild(pause_text);
    AddChild(m_quit_text);
    AddChild(m_close_text);
    AddChild(m_input_layout);

    m_aborted_hash = hash::Hash(aborted_hash.c_str());
}

void PauseScreen::ShowAt(const math::Vector& position)
{
    m_position = position;

    const UINavigationSetup quit_nav_setup = {
        mono::INVALID_ID, m_close_proxy.GetEntityId(), mono::INVALID_ID, mono::INVALID_ID
    };
    m_quit_proxy.UpdateUIItem(m_quit_text->Transform(), m_quit_text->BoundingBox(), m_aborted_hash, quit_nav_setup);

    const UINavigationSetup close_nav_setup = {
        m_quit_proxy.GetEntityId(), mono::INVALID_ID, mono::INVALID_ID, mono::INVALID_ID
    };
    m_close_proxy.UpdateUIItem(m_close_text->Transform(), m_close_text->BoundingBox(), hash::Hash("close"), close_nav_setup);

    const mono::InputContextType most_recent_input = m_input_system->GetMostRecentGlobalInput();
    const char* input_layout_texture =
        (most_recent_input == mono::InputContextType::Controller) ? 
            "res/textures/gamepad/gamepad_button_layout.png" : "res/textures/gamepad/keyboard_layout.png";
    m_input_layout->SetTexture(input_layout_texture, 1.0f);

    Show();
}

void PauseScreen::Show()
{
    m_quit_proxy.SetItemState(UIItemState::Enabled);
    m_close_proxy.SetItemState(UIItemState::Enabled);

    UIElement::Show();
    m_ui_system->Enable();
}

void PauseScreen::Hide()
{
    m_quit_proxy.SetItemState(UIItemState::Disabled);
    m_close_proxy.SetItemState(UIItemState::Disabled);

    UIElement::Hide();
    m_ui_system->Disable();
}

math::Quad PauseScreen::BoundingBox() const
{
    return math::InfQuad;
}
