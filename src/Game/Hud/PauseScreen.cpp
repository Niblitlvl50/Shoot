
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

using namespace game;

PauseScreen::PauseScreen(
    mono::TransformSystem* transform_system,
    mono::InputSystem* input_system,
    mono::IEntityManager* entity_manager,
    game::UISystem* ui_system)
    //: game::UIOverlay(200, 200.0f / mono::GetWindowAspect())
    : m_transform_system(transform_system)
    , m_input_system(input_system)
    , m_entity_manager(entity_manager)
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
    const math::Vector close_text_size = mono::MeasureString(font_id, close_text);

    m_close_text =
        new UITextElement(font_id, close_text, mono::FontCentering::DEFAULT_CENTER, mono::Color::GRAY);
    m_close_text->SetPosition(background_x + background_width - close_text_size.x - 0.2f, background_y + 0.15f);

    UITextureElement* texture_element = new UITextureElement("res/textures/gamepad/gamepad_button_layout.png");
    texture_element->SetScale(0.0075f);

    AddChild(background_element);
    AddChild(pause_text);
    AddChild(m_quit_text);
    AddChild(m_close_text);
    AddChild(texture_element);
}

void PauseScreen::ShowAt(const math::Vector& position)
{
    m_position = position;

    const math::Matrix& quit_text_transform = Transform() * m_quit_text->Transform();
    m_quit_proxy.UpdateUIItem(quit_text_transform, m_quit_text->GetBounds());

    const math::Matrix& close_text_transform = Transform() * m_close_text->Transform();
    m_close_proxy.UpdateUIItem(close_text_transform, m_close_text->GetBounds());

    // m_entities = m_entity_manager->CreateEntityCollection("res/entities/pause_screen_collection.entity");
    // math::Matrix& transform = m_transform_system->GetTransform(m_entities.front().id);
    // math::Position(transform, position);

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

    // m_entity_manager->ReleaseEntities(m_entities);
    // m_entities.clear();
}
