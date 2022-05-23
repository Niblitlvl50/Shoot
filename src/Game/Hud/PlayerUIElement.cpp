
#include "PlayerUIElement.h"
#include "Player/PlayerInfo.h"
#include "FontIds.h"
#include "UIElements.h"
#include "Resources.h"

#include "Math/EasingFunctions.h"

#include "EventHandler/EventHandler.h"
#include "Events/PlayerEvents.h"

#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Sprite/Sprite.h"
#include "Rendering/Sprite/SpriteFactory.h"
#include "Rendering/RenderSystem.h"

#include "System/System.h"
#include "StateMachine.h"

#include <cstdio>
#include <unordered_map>

namespace game
{
    constexpr float g_player_element_width = 2.5f;
    constexpr float g_player_element_half_width = g_player_element_width / 2.0f;
    constexpr float g_player_death_element_width = 3.0f;
    constexpr float g_player_element_height = 1.0f;

    class PlayerElement : public game::UIElement
    {
    public:

        static constexpr float transision_duration_s = 0.5f;

        PlayerElement(
            const PlayerInfo& player_info,
            const math::Vector& onscreen_position,
            const math::Vector& offscreen_position,
            mono::SpriteSystem* sprite_system)
            : m_player_info(player_info)
            , m_sprite_system(sprite_system)
            , m_timer(0.0f)
        {
            m_position = m_offscreen_position = offscreen_position;
            m_screen_position = onscreen_position;

            UISpriteElement* background_hud = new UISpriteElement("res/sprites/player_background_hud.sprite");
            background_hud->SetPosition(0.0f, 0.5f);

            m_mugshot_hud = new UISpriteElement();
            m_mugshot_hud->SetPosition(-0.6f, 0.5f);

            std::vector<std::string> weapon_sprites;

            const std::vector<game::WeaponSetup> weapon_list = game::GetWeaponList();
            for(uint32_t index = 0; index < weapon_list.size(); ++index)
            {
                const game::WeaponSetup& weapon_setup = weapon_list[index];
                const char* weapon_sprite = game::GetWeaponSpriteFromHash(weapon_setup.weapon_hash);
                weapon_sprites.push_back(weapon_sprite);

                m_weapon_hash_to_index[weapon_setup.weapon_hash] = index;
            }

            m_weapon_sprites = new UISpriteElement(weapon_sprites);
            m_weapon_sprites->SetPosition(0.25f, 0.45f);

            m_ammo_text = new UITextElement(
                FontId::RUSSOONE_TINY, "", mono::FontCentering::HORIZONTAL_VERTICAL, mono::Color::MAGENTA);
            m_ammo_text->SetPosition(0.5f, 0.35f);
            m_ammo_text->SetScale(0.5f);

            AddChild(background_hud);
            AddChild( m_mugshot_hud);
            AddChild(m_weapon_sprites);
            AddChild(m_ammo_text);

            const PlayerUIStateMachine::StateTable states = {
                PlayerUIStateMachine::MakeState(States::APPEAR, &PlayerElement::ToAppear, &PlayerElement::Appearing, this),
                PlayerUIStateMachine::MakeState(States::ACTIVE, &PlayerElement::ToActive, &PlayerElement::Active, this),
                PlayerUIStateMachine::MakeState(States::DISAPPEAR, &PlayerElement::ToDisappear, &PlayerElement::Disappearing, this)
            };
            m_states.SetStateTableAndState(states, States::DISAPPEAR);
        }

        void Update(const mono::UpdateContext& update_context) override
        {
            UIElement::Update(update_context);
            m_states.UpdateState(update_context.delta_s);
            m_timer = std::clamp(m_timer, 0.0f, transision_duration_s);
            AnimatePlayerElement(m_timer);
        }

        void ToAppear()
        {
            const mono::Sprite* player_sprite = m_sprite_system->GetSprite(m_player_info.entity_id);
            const char* sprite_filename = game::HashToFilename(player_sprite->GetSpriteHash());
            m_mugshot_hud->SetSprite(sprite_filename);
        }
        void Appearing(const float& delta_s)
        {
            m_timer += delta_s;
            if(m_timer >= transision_duration_s)
                m_states.TransitionTo(States::ACTIVE);
        }

        void ToActive()
        { }
        void Active(const float& delta_s)
        {
            char ammo_text[32] = { '\0' };
            std::snprintf(ammo_text, std::size(ammo_text), "%2u", m_player_info.magazine_left);
            m_ammo_text->SetText(ammo_text);

            const uint32_t weapon_index = m_weapon_hash_to_index[m_player_info.weapon_type.weapon_hash];
            m_weapon_sprites->SetActiveSprite(weapon_index);

            if(m_player_info.player_state == PlayerState::NOT_SPAWNED)
                m_states.TransitionTo(States::DISAPPEAR);
        }

        void ToDisappear()
        { }
        void Disappearing(const float& delta_s)
        {
            m_timer -= delta_s;
            if(m_player_info.player_state == PlayerState::ALIVE)
                m_states.TransitionTo(States::APPEAR);
        }

        void AnimatePlayerElement(float time)
        {
            m_position.x = math::EaseInCubic(
                time, transision_duration_s, m_offscreen_position.x, m_screen_position.x - m_offscreen_position.x);
            m_position.y = math::EaseInCubic(
                time, transision_duration_s, m_offscreen_position.y, m_screen_position.y - m_offscreen_position.y);
        }

        const PlayerInfo& m_player_info;
        mono::SpriteSystem* m_sprite_system;

        math::Vector m_screen_position;
        math::Vector m_offscreen_position;
        float m_timer;

        std::unordered_map<uint32_t, uint32_t> m_weapon_hash_to_index;
        class UITextElement* m_ammo_text;
        class UISpriteElement* m_mugshot_hud;
        class UISpriteElement* m_weapon_sprites;

        enum class States
        {
            APPEAR,
            DISAPPEAR,
            ACTIVE
        };

        using PlayerUIStateMachine = StateMachine<States, float>;
        PlayerUIStateMachine m_states;
    };


    class PlayerDeathElement : public game::UIElement
    {
    public:
    
        PlayerDeathElement(
            const PlayerInfo& player_info,
            mono::EventHandler* event_handler,
            const math::Vector& onscreen_position,
            const math::Vector& offscreen_position)
            : m_player_info(player_info)
            , m_event_handler(event_handler)
            , m_timer(0)
        {
            m_position = m_offscreen_position = offscreen_position;
            m_screen_position = onscreen_position;

            UISquareElement* background = new UISquareElement(
                g_player_death_element_width, g_player_element_height, mono::Color::RGBA(0.0f, 0.0f, 0.0f, 0.8f), mono::Color::BLACK, 1.0f);

            UISpriteElement* skull_sprite = new UISpriteElement("res/sprites/skull_red_eyes1.sprite");
            skull_sprite->SetPosition(0.4f, 0.5f);

            UITextElement* death_text = new UITextElement(
                FontId::RUSSOONE_TINY, "You are dead!", mono::FontCentering::HORIZONTAL_VERTICAL, mono::Color::OFF_WHITE);
            death_text->SetPosition(1.75f, 0.60f);
            death_text->SetScale(0.7f);

            UITextElement* death_text_2 = new UITextElement(
                FontId::RUSSOONE_TINY, "Continue?", mono::FontCentering::HORIZONTAL_VERTICAL, mono::Color::OFF_WHITE);
            death_text_2->SetPosition(1.75f, 0.35f);
            death_text_2->SetScale(0.7f);

            AddChild(background);
            AddChild(skull_sprite);
            AddChild(death_text);
            AddChild(death_text_2);
        }

        void Update(const mono::UpdateContext& update_context) override
        {
            UIElement::Update(update_context);

            UpdateAnimation(update_context);

            if(m_player_info.player_state == game::PlayerState::DEAD)
            {
                const System::ControllerState& state = System::GetController(System::ControllerId(m_player_info.controller_id));
                const bool respawn_pressed = System::ButtonTriggeredAndChanged(m_last_state.button_state, state.button_state, System::ControllerButton::FACE_BOTTOM);
                const bool quit_pressed = System::ButtonTriggeredAndChanged(m_last_state.button_state, state.button_state, System::ControllerButton::FACE_TOP);

                m_last_state = state;

                if(respawn_pressed)
                {
                    m_event_handler->DispatchEvent(game::RespawnPlayerEvent(m_player_info.entity_id));
                }
                else if(quit_pressed)
                {
                    //m_states.TransitionTo(GameModeStates::FADE_OUT);
                }
            }
        }

        void UpdateAnimation(const mono::UpdateContext& update_context)
        {
            constexpr float transision_duration_s = 0.5f;

            switch(m_player_info.player_state)
            {
            case game::PlayerState::NOT_SPAWNED:
            case game::PlayerState::ALIVE:
            {
                if(m_timer > 0.0f)
                {
                    m_position.x = math::EaseOutCubic(m_timer, transision_duration_s, m_offscreen_position.x, m_screen_position.x - m_offscreen_position.x);
                    m_position.y = math::EaseOutCubic(m_timer, transision_duration_s, m_offscreen_position.y, m_screen_position.y - m_offscreen_position.y);
                    m_timer -= update_context.delta_s;
                }
                
                break;
            }
            case game::PlayerState::DEAD:
            {
                if(m_timer < transision_duration_s)
                {
                    m_position.x = math::EaseOutCubic(m_timer, transision_duration_s, m_offscreen_position.x, m_screen_position.x - m_offscreen_position.x);
                    m_position.y = math::EaseOutCubic(m_timer, transision_duration_s, m_offscreen_position.y, m_screen_position.y - m_offscreen_position.y);
                    m_timer += update_context.delta_s;
                }

                break;
            }
            }

            m_timer = std::clamp(m_timer, 0.0f, transision_duration_s);
        }

        const PlayerInfo& m_player_info;
        mono::EventHandler* m_event_handler;

        float m_timer;
        math::Vector m_screen_position;
        math::Vector m_offscreen_position;

        System::ControllerState m_last_state;
   };
}

using namespace game;

PlayerUIElement::PlayerUIElement(const PlayerInfo* player_infos, int num_players, mono::SpriteSystem* sprite_system, mono::EventHandler* event_handler)
    : UIOverlay(12.0f, 12.0f / mono::GetWindowAspect())
{
    const float position_x = m_width - g_player_element_half_width;

    const math::Vector hud_positions[] = {
        { g_player_element_half_width, 0.0f },
        { position_x, 0.0f },
        { m_width / 2.0f, 0.0f },
    };

    const math::Vector hud_offscreen_delta[] = {
        { -g_player_element_width, 0.0f },
        { +g_player_element_width, 0.0f },
        { 0.0f, -2.0f },
    };

    const math::Vector death_element_offset[] = {
        { (-g_player_death_element_width / 2.0f) + 0.25f, 0.0f },
        { (-g_player_death_element_width / 2.0f) - 0.25f, 0.0f },
        { (-g_player_death_element_width / 2.0f), 0.0f },
    };

    const math::Vector death_element_offscreen_delta[] = {
        { -g_player_death_element_width, 0.0f },
        { g_player_death_element_width, 0.0f },
        { 0.0f, -2.0f },
    };

    for(int index = 0; index < num_players; ++index)
    {
        const PlayerInfo& player_info = player_infos[index];

        const math::Vector on_screen_position = hud_positions[index];
        const math::Vector off_screen_position = on_screen_position + hud_offscreen_delta[index];
        AddChild(new PlayerElement(player_info, on_screen_position, off_screen_position, sprite_system));

        const math::Vector on_screen_death_position = on_screen_position + death_element_offset[index];
        const math::Vector off_screen_death_position = on_screen_death_position + death_element_offscreen_delta[index];
        AddChild(new PlayerDeathElement(player_info, event_handler, on_screen_death_position, off_screen_death_position));
    }
}
