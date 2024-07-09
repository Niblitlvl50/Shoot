
#include "LevelUpScreen.h"

#include "FontIds.h"
#include "UI/UISystem.h"

#include "EntitySystem/Entity.h"
#include "Input/InputSystem.h"
#include "Rendering/Text/TextFunctions.h"
#include "TransformSystem/TransformSystem.h"

using namespace game;

LevelUpScreen::LevelUpScreen(
    mono::TransformSystem* transform_system,
    mono::IEntityManager* entity_manager,
    mono::EventHandler* event_handler,
    UISystem* ui_system)
    : m_ui_system(ui_system)
    , m_close_proxy(ui_system, transform_system, entity_manager)
{
    const float background_width = 2.5f;
    const float background_height = 2.5f;
    const float background_half_width = background_width / 2.0f;
    const float background_half_height = background_height / 2.0f;

    const FontId font_id = FontId::RUSSOONE_TINY;

    UITextElement* title_text = new UITextElement(font_id, "LEVEL UP", mono::Color::GRAY);
    title_text->SetPosition(background_half_width, background_height - 0.75f);

    const char* close_text = "Close";
    const mono::TextMeasurement close_text_measurement = mono::MeasureString(font_id, close_text);
    m_close_text = new UITextElement(font_id, close_text, mono::Color::GRAY);
    m_close_text->SetPosition(background_width - close_text_measurement.size.x - 0.25f, 0.25f);

    UISquareElement* background_element =
        new UISquareElement(background_width, background_height, mono::Color::BLACK, mono::Color::GRAY, 1.0f);
    background_element->SetPosition(-background_half_width - 2.5f, -background_half_height + 0.25);
    background_element->AddChild(title_text);
    background_element->AddChild(m_close_text);

    m_close_proxy.SetItemState(UIItemState::Disabled);

    AddChild(background_element);
}

void LevelUpScreen::ShowAt(const math::Vector& position)
{
    m_position = position;

    const UINavigationSetup close_nav_setup = {
        mono::INVALID_ID, mono::INVALID_ID, mono::INVALID_ID, mono::INVALID_ID
    };
    m_close_proxy.UpdateUIItem(m_close_text->Transform(), m_close_text->BoundingBox(), "close", close_nav_setup);

    const auto close_callback = [this](uint32_t entity_id) {
        Hide();
    };
    m_close_proxy.SetItemCallback(close_callback);

    Show();
}

void LevelUpScreen::Show()
{
    m_close_proxy.SetItemState(UIItemState::Enabled);

    UIElement::Show();
    m_ui_system->Enable();
}

void LevelUpScreen::Hide()
{
    m_close_proxy.SetItemState(UIItemState::Disabled);

    UIElement::Hide();
    m_ui_system->Disable();
}
