
#include "ShopScreen.h"

#include "FontIds.h"
#include "ShopSystem/ShopSystem.h"
#include "UI/UISystem.h"

#include "EntitySystem/Entity.h"
#include "Input/InputSystem.h"
#include "Rendering/Text/TextFunctions.h"
#include "TransformSystem/TransformSystem.h"

using namespace game;

ShopScreen::ShopScreen(
    mono::TransformSystem* transform_system,
    mono::IEntityManager* entity_manager,
    mono::EventHandler* event_handler,
    UISystem* ui_system,
    ShopSystem* shop_system)
    : m_ui_system(ui_system)
    , m_shop_system(shop_system)
    , m_close_proxy(ui_system, transform_system, entity_manager)
{
    const float background_width = 5.5f;
    const float background_height = 5.5f;
    const float background_half_width = background_width / 2.0f;
    const float background_half_height = background_height / 2.0f;

    const FontId font_id = FontId::RUSSOONE_TINY;

    UISpriteElement* shopkeeper_sprite = new UISpriteElement("res/sprites/cactus_small.sprite");
    shopkeeper_sprite->SetPosition(0.75f, background_height - 0.75f);
    shopkeeper_sprite->SetScale(2.0f);
    shopkeeper_sprite->SetActiveSprite(0, 1);

    UITextElement* title_text = new UITextElement(font_id, "SHOP", mono::Color::GRAY);
    title_text->SetPosition(background_half_width, background_height - 0.75f);

    UISquareElement* item_background = new UISquareElement(3.5f, 3.5f, mono::Color::MakeWithAlpha(mono::Color::GRAY, 0.2f));
    item_background->SetPosition(1.5f, 1.0f);

    const char* close_text = "Close";
    const mono::TextMeasurement close_text_measurement = mono::MeasureString(font_id, close_text);
    m_close_text = new UITextElement(font_id, close_text, mono::Color::GRAY);
    m_close_text->SetPosition(background_width - close_text_measurement.size.x - 0.25f, 0.25f);

    UISquareElement* background_element =
        new UISquareElement(background_width, background_height, mono::Color::BLACK, mono::Color::GRAY, 1.0f);
    background_element->SetPosition(-background_half_width - 2.5f, -background_half_height + 0.25);
    background_element->AddChild(shopkeeper_sprite);
    background_element->AddChild(title_text);
    background_element->AddChild(item_background);
    background_element->AddChild(m_close_text);

    m_close_proxy.SetItemState(UIItemState::Disabled);

    AddChild(background_element);
}

void ShopScreen::ShowAt(const math::Vector& position)
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

void ShopScreen::Show()
{
    m_close_proxy.SetItemState(UIItemState::Enabled);

    UIElement::Show();
    m_ui_system->Enable();
}

void ShopScreen::Hide()
{
    m_close_proxy.SetItemState(UIItemState::Disabled);

    UIElement::Hide();
    m_ui_system->Disable();
}
