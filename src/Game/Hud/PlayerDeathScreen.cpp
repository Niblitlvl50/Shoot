
#include "PlayerDeathScreen.h"

#include "Player/PlayerInfo.h"
#include "FontIds.h"
#include "UIElements.h"
#include "Events/PlayerConnectedEvent.h"

#include "EventHandler/EventHandler.h"
#include "Events/QuitEvent.h"
#include "Math/EasingFunctions.h"
#include "System/System.h"

#include "Rendering/RenderSystem.h"

using namespace game;

PlayerDeathScreen::PlayerDeathScreen(const PlayerInfo& player_info, mono::EventHandler* event_handler)
    : UIOverlay(1200.0f, 1200.0f / mono::GetWindowAspect())
    , m_player_info(player_info)
    , m_event_handler(event_handler)
    , m_timer(0)
{
    const float half_width = m_width / 2.0f;
    const float half_height = m_height / 2.0f;
    const float square_height = m_height * 0.3f;

    m_position = m_offscreen_position = math::Vector(-half_width, half_height);
    m_screen_position = math::Vector(half_width, half_height);

    UISquareElement* background = new UISquareElement(m_width, square_height, mono::Color::RGBA(0.2f, 0.2f, 0.2f, 0.5f), mono::Color::BLACK, 1.0f);
    background->SetPosition(math::Vector(-half_width, -(square_height / 2.0f)));

    UITextElement* gameover_text = new UITextElement(
        shared::FontId::RUSSOONE_MEGA, "You are Dead!", mono::FontCentering::HORIZONTAL_VERTICAL, mono::Color::BLACK);
    gameover_text->SetPosition(math::Vector(0.0f, 20.0f));

    UITextElement* continue_text = new UITextElement(
        shared::FontId::RUSSOONE_LARGE, "Press button to continue", mono::FontCentering::HORIZONTAL_VERTICAL, mono::Color::RGBA(0.7f, 0.7f, 0.7f));
    continue_text->SetPosition(math::Vector(0.0f, -40.0f));

    AddChild(background);
    AddChild(gameover_text);
    AddChild(continue_text);
}

void PlayerDeathScreen::Update(const mono::UpdateContext& update_context)
{
    UIOverlay::Update(update_context);

    switch(m_player_info.player_state)
    {
    case game::PlayerState::NOT_SPAWNED:
        break;

    case game::PlayerState::ALIVE:
    {
        if(m_timer > 0.0f)
        {
            m_position.x =
                math::EaseInCubic(m_timer, 1.0f, m_offscreen_position.x, m_screen_position.x - m_offscreen_position.x);
            m_timer -= update_context.delta_s;
        }

        break;
    }

    case game::PlayerState::DEAD:
    {
        if(m_timer < 1.0f)
        {
            m_position.x =
                math::EaseOutCubic(m_timer, 1.0f, m_offscreen_position.x, m_screen_position.x - m_offscreen_position.x);
            m_timer += update_context.delta_s;
        }

        const System::ControllerState& state = System::GetController(System::ControllerId::Primary);
        const bool a_pressed = System::ButtonTriggeredAndChanged(m_last_state.button_state, state.button_state, System::ControllerButton::A);
        const bool y_pressed = System::ButtonTriggeredAndChanged(m_last_state.button_state, state.button_state, System::ControllerButton::Y);

        if(a_pressed)
            m_event_handler->DispatchEvent(game::RespawnPlayerEvent(m_player_info.entity_id));
        else if(y_pressed)
            m_event_handler->DispatchEvent(event::QuitEvent());

        m_last_state = state;
        break;
    }
    };
}


BigTextScreen::BigTextScreen(
    const char* text,
    const char* sub_text,
    const mono::Color::RGBA& color,
    const mono::Color::RGBA& border_color,
    const mono::Color::RGBA& text_color,
    const mono::Color::RGBA& subtext_color)
    : UIOverlay(1200.0f, 1200.0f / mono::GetWindowAspect())
{
    const float half_width = m_width / 2.0f;
    const float half_height = m_height / 2.0f;
    const float square_height = m_height * 0.3f;

    m_background = new UISquareElement(m_width, m_height, color, border_color, 1.0f);

    m_main_text_element = new UITextElement(
        shared::FontId::RUSSOONE_MEGA, text, mono::FontCentering::HORIZONTAL_VERTICAL, text_color);
    m_main_text_element->SetPosition(math::Vector(half_width, half_height + 20.0f));

    m_sub_text_element = new UITextElement(
        shared::FontId::RUSSOONE_LARGE, sub_text, mono::FontCentering::HORIZONTAL_VERTICAL, subtext_color);
    m_sub_text_element->SetPosition(math::Vector(half_width, half_height - 40.0f));

    AddChild(m_background);
    AddChild(m_main_text_element);
    AddChild(m_sub_text_element);
}

void BigTextScreen::Update(const mono::UpdateContext& update_context)
{
    UIOverlay::Update(update_context);
}

void BigTextScreen::SetText(const char* text)
{
    m_main_text_element->SetText(text);
}

void BigTextScreen::SetSubText(const char* sub_text)
{
    m_sub_text_element->SetText(sub_text);
}

void BigTextScreen::SetAlpha(float alpha)
{
    mono::Color::RGBA color = m_background->GetColor();
    color.alpha = alpha;

    mono::Color::RGBA border_color = m_background->GetBorderColor();
    border_color.alpha = alpha;

    m_background->SetColor(color);
    m_background->SetBorderColor(border_color);

    m_main_text_element->SetAlpha(alpha);
    m_sub_text_element->SetAlpha(alpha);
}
