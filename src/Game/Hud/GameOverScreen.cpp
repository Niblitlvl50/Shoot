
#include "GameOverScreen.h"

#include "AIKnowledge.h"
#include "FontIds.h"
#include "UIElements.h"
#include "Events/PlayerConnectedEvent.h"

#include "EventHandler/EventHandler.h"
#include "Events/QuitEvent.h"
#include "Math/EasingFunctions.h"
#include "System/System.h"

using namespace game;

GameOverScreen::GameOverScreen(
    const PlayerInfo& player_info,
    const math::Vector& position,
    const math::Vector& offscreen_position,
    mono::SpriteSystem* sprite_system,
    mono::IEntityManager* entity_manager,
    mono::EventHandler* event_handler)
    : m_player_info(player_info)
    , m_screen_position(position)
    , m_offscreen_position(offscreen_position)
    , m_event_handler(event_handler)
    , m_timer(0)
{
    m_position = offscreen_position;

    UISquareElement* background = new UISquareElement(math::Quad(-150.0f, -40.0f, 300.0f, 40.0f), mono::Color::RGBA(0.2f, 0.2f, 0.2f, 0.5f));

    UITextElement* gameover_text = new UITextElement(shared::FontId::PIXELETTE_MEGA, "Game Over!", true, mono::Color::BLACK);
    //gameover_text->SetPosition(math::Vector(0.0f, 20.0f));
    //gameover_text->SetScale(math::Vector(3.0f, 3.0f));

    UITextElement* continue_text = new UITextElement(shared::FontId::PIXELETTE_MEGA, "Press X to continue", true, mono::Color::BLACK);
    //continue_text->SetPosition(math::Vector(0.0f, 0.0f));
    //continue_text->SetScale(math::Vector(2.0f, 2.0f));

    const std::vector<std::string>& sprite_files = {
        "res/sprites/ps_cross.sprite"
    };
    UISpriteElement* a_button_sprite = new UISpriteElement(sprite_files, sprite_system, entity_manager);
    //a_button_sprite->SetPosition(math::Vector(0.0f, -20.0f));
    //a_button_sprite->SetScale(math::Vector(10.0f, 10.0f));


    //AddChild(background);
    //AddChild(gameover_text);
    //AddChild(continue_text);
    //AddChild(a_button_sprite);
}

void GameOverScreen::Update(const mono::UpdateContext& update_context)
{
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
            m_timer -= float(update_context.delta_ms) / 1000.0f;
        }

        break;
    }

    case game::PlayerState::DEAD:
    {
        if(m_timer < 1.0f)
        {
            m_position.x =
                math::EaseOutCubic(m_timer, 1.0f, m_offscreen_position.x, m_screen_position.x - m_offscreen_position.x);
            m_timer += float(update_context.delta_ms) / 1000.0f;
        }

        const System::ControllerState& state = System::GetController(System::ControllerId::Primary);
        const bool a_pressed = System::ButtonTriggeredAndChanged(m_last_state.button_state, state.button_state, System::ControllerButton::A);
        const bool y_pressed = System::ButtonTriggeredAndChanged(m_last_state.button_state, state.button_state, System::ControllerButton::Y);

        if(a_pressed)
            m_event_handler->DispatchEvent(game::SpawnPlayerEvent());
        else if(y_pressed)
            m_event_handler->DispatchEvent(event::QuitEvent());

        m_last_state = state;
        break;
    }
    };
}
