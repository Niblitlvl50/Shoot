
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

#define IS_TRIGGERED(variable) (!m_last_state.variable && state.variable)
#define HAS_CHANGED(variable) (m_last_state.variable != state.variable)


GameOverScreen::GameOverScreen(
    const PlayerInfo& player_info, const math::Vector& position, const math::Vector& offscreen_position, mono::EventHandler* event_handler)
    : m_player_info(player_info)
    , m_screen_position(position)
    , m_offscreen_position(offscreen_position)
    , m_event_handler(event_handler)
    , m_timer(0)
{
    m_position = offscreen_position;

    UITextElement* m_gameover_text = new UITextElement(shared::FontId::PIXELETTE_MEGA, "Game Over!", true, mono::Color::BLACK);
    m_gameover_text->SetScale(math::Vector(3.0f, 3.0f));

    AddChild(m_gameover_text);
}

void GameOverScreen::EntityDraw(mono::IRenderer& renderer) const
{ }

void GameOverScreen::EntityUpdate(const mono::UpdateContext& update_context)
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
        const bool a_pressed = IS_TRIGGERED(a) && HAS_CHANGED(a);
        const bool y_pressed = IS_TRIGGERED(y) && HAS_CHANGED(y);

        if(a_pressed)
            m_event_handler->DispatchEvent(game::SpawnPlayerEvent());
        else if(y_pressed)
            m_event_handler->DispatchEvent(event::QuitEvent());

        m_last_state = state;
        break;
    }
    };
}
