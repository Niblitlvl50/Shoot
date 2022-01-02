
#include "PlayerUIElement.h"

#include "Player/PlayerInfo.h"
#include "FontIds.h"
#include "UIElements.h"
#include "Math/EasingFunctions.h"

#include "EventHandler/EventHandler.h"
#include "Events/PlayerEvents.h"

#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"
#include "Rendering/Sprite/ISprite.h"
#include "Rendering/Sprite/SpriteFactory.h"
#include "Rendering/RenderSystem.h"

#include "System/System.h"

#include <cstdio>
#include <unordered_map>

namespace game
{
    constexpr float g_player_element_width = 5.5f;
    constexpr float g_player_death_element_width = 5.0f;
    constexpr float g_player_element_height = 1.0f;

    class HeartContainer : public game::UIElement
    {
    public:

        static constexpr const int max_lives = 3;
        static constexpr const char* heart_sprite = "res/sprites/heart.sprite";

        HeartContainer(const PlayerInfo& player_info)
            : m_player_info(player_info)
        {
            for(size_t index = 0; index < std::size(m_hearts); ++index)
            {
                game::UISpriteElement* element = new game::UISpriteElement(heart_sprite);
                element->SetPosition(math::Vector(index * 0.8f, 0.0f));
                m_hearts[index] = element;
                AddChild(element);
            }

            mono::ISprite* sprite = m_hearts[0]->GetSprite(0);
            m_deafault_id = sprite->GetAnimationIdFromName("default");
            m_dead_id = sprite->GetAnimationIdFromName("dead");
        }

        void Update(const mono::UpdateContext& update_context) override
        {
            UIElement::Update(update_context);
            SetLives(m_player_info.lives);
        }

        void SetLives(int lives)
        {
            for(int index = 0; index < max_lives; ++index)
            {
                const mono::Color::RGBA shade = (index < lives) ? mono::Color::WHITE : mono::Color::GRAY;
                const int animation =
                    (((index +1) == lives) && m_player_info.player_state == PlayerState::ALIVE) ? m_deafault_id : m_dead_id;

                mono::ISprite* sprite = m_hearts[index]->GetSprite(0);
                sprite->SetShade(shade);

                if(animation != sprite->GetActiveAnimation())
                    sprite->SetAnimation(animation);
            }
        }

        const PlayerInfo& m_player_info;
        game::UISpriteElement* m_hearts[max_lives];
        int m_deafault_id;
        int m_dead_id;
    };

    class PlayerElement : public game::UIElement
    {
    public:

        PlayerElement(const PlayerInfo& player_info, const math::Vector& onscreen_position, const math::Vector& offscreen_position)
            : m_player_info(player_info)
            , m_timer(0)
        {
            m_position = m_offscreen_position = offscreen_position;
            m_screen_position = onscreen_position;

            UISquareElement* background = new UISquareElement(
                g_player_element_width, g_player_element_height, mono::Color::RGBA(0.2f, 0.2f, 0.2f, 0.5f), mono::Color::BLACK, 1.0f);

            HeartContainer* hearts = new HeartContainer(player_info);
            hearts->SetPosition(0.5f, 0.5f);

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
            m_weapon_sprites->SetPosition(3.5f, 0.5f);

            m_ammo_text = new UITextElement(shared::FontId::PIXELETTE_TINY, "", mono::FontCentering::HORIZONTAL_VERTICAL, mono::Color::MAGENTA);
            m_ammo_text->SetPosition(4.5f, 0.5f);

            AddChild(background);
            AddChild(hearts);
            AddChild(m_weapon_sprites);
            AddChild(m_ammo_text);
        }

        void Update(const mono::UpdateContext& update_context) override
        {
            UIElement::Update(update_context);

            char ammo_text[32] = { '\0' };
            std::snprintf(ammo_text, std::size(ammo_text), "%2u", m_player_info.magazine_left);
            m_ammo_text->SetText(ammo_text);

            const uint32_t weapon_index = m_weapon_hash_to_index[m_player_info.weapon_type.weapon_hash];
            m_weapon_sprites->SetActiveSprite(weapon_index);

            constexpr float transision_duration_s = 0.5f;

            if(m_player_info.player_state != game::PlayerState::NOT_SPAWNED && m_timer < transision_duration_s)
            {
                m_position.x = math::EaseOutCubic(m_timer, transision_duration_s, m_offscreen_position.x, m_screen_position.x - m_offscreen_position.x);
                m_position.y = math::EaseOutCubic(m_timer, transision_duration_s, m_offscreen_position.y, m_screen_position.y - m_offscreen_position.y);
                m_timer += update_context.delta_s;
            }
            else if(m_player_info.player_state == game::PlayerState::NOT_SPAWNED && m_timer > 0.0f)
            {
                m_position.x = math::EaseInCubic(m_timer, transision_duration_s, m_offscreen_position.x, m_screen_position.x - m_offscreen_position.x);
                m_position.y = math::EaseInCubic(m_timer, transision_duration_s, m_offscreen_position.y, m_screen_position.y - m_offscreen_position.y);
                m_timer -= update_context.delta_s;
            }

            m_timer = std::clamp(m_timer, 0.0f, transision_duration_s);
        }

        const PlayerInfo& m_player_info;

        math::Vector m_screen_position;
        math::Vector m_offscreen_position;
        float m_timer;

        std::unordered_map<uint32_t, uint32_t> m_weapon_hash_to_index;
        class UITextElement* m_ammo_text;
        class UISpriteElement* m_weapon_sprites;
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
            skull_sprite->SetPosition(0.5f, 0.5f);

            UITextElement* death_text = new UITextElement(
                shared::FontId::PIXELETTE_TINY, "You are dead!", mono::FontCentering::HORIZONTAL_VERTICAL, mono::Color::OFF_WHITE);
            death_text->SetPosition(2.5f, 0.65f);

            UITextElement* death_text_2 = new UITextElement(
                shared::FontId::PIXELETTE_TINY, "Continue?", mono::FontCentering::HORIZONTAL_VERTICAL, mono::Color::OFF_WHITE);
            death_text_2->SetPosition(2.5f, 0.35f);

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
                const bool a_pressed = System::ButtonTriggeredAndChanged(m_last_state.button_state, state.button_state, System::ControllerButton::A);
                const bool y_pressed = System::ButtonTriggeredAndChanged(m_last_state.button_state, state.button_state, System::ControllerButton::Y);

                m_last_state = state;

                if(a_pressed)
                {
                    m_event_handler->DispatchEvent(game::RespawnPlayerEvent(m_player_info.entity_id));
                }
                else if(y_pressed)
                {
                    //m_states.TransitionTo(GameModeStates::FADE_OUT);
                }
            }
        }

        void UpdateAnimation(const mono::UpdateContext& update_context)
        {
            constexpr float transision_duration_s = 0.5f;

            if(m_player_info.player_state == game::PlayerState::ALIVE && m_timer > 0.0f)
            {
                m_position.x = math::EaseOutCubic(m_timer, transision_duration_s, m_offscreen_position.x, m_screen_position.x - m_offscreen_position.x);
                m_position.y = math::EaseOutCubic(m_timer, transision_duration_s, m_offscreen_position.y, m_screen_position.y - m_offscreen_position.y);
                m_timer -= update_context.delta_s;
            }
            else if(m_player_info.player_state == game::PlayerState::DEAD && m_timer < transision_duration_s)
            {
                m_position.x = math::EaseOutCubic(m_timer, transision_duration_s, m_offscreen_position.x, m_screen_position.x - m_offscreen_position.x);
                m_position.y = math::EaseOutCubic(m_timer, transision_duration_s, m_offscreen_position.y, m_screen_position.y - m_offscreen_position.y);
                m_timer += update_context.delta_s;
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

PlayerUIElement::PlayerUIElement(const PlayerInfo* player_infos, int n_players, mono::EventHandler* event_handler)
    : UIOverlay(16.0f, 16.0f / mono::GetWindowAspect())
{
    const float position_x = m_width - g_player_element_width;
    const float position_y = m_height - g_player_element_height;

    const math::Vector hud_positions[] = {
        { 0.0f, 0.0f },
        { position_x, 0.0f },
        { 0.0f, position_y },
        { position_x, position_y }
    };

    const math::Vector hud_offscreen_delta_positions[] = {
        { -g_player_element_width, 0.0f },
        { +g_player_element_width, 0.0f },
        { -g_player_element_width, 0.0f },
        { +g_player_element_width, 0.0f }
    };

    const math::Vector hud_death_element_offset[] = {
        { 0.0f, +0.25f },
        { 0.5f, +0.25f },
        { 0.0f, -0.25f },
        { 0.5f, -0.25f }
    };

    for(int index = 0; index < n_players; ++index)
    {
        const math::Vector on_screen_position = hud_positions[index];
        const math::Vector off_screen_position = on_screen_position + hud_offscreen_delta_positions[index];
        AddChild(new PlayerElement(player_infos[index], on_screen_position, off_screen_position));

        const math::Vector on_screen_death_position = on_screen_position + hud_death_element_offset[index];
        const math::Vector off_screen_death_position = off_screen_position + hud_death_element_offset[index];
        AddChild(new PlayerDeathElement(player_infos[index], event_handler, on_screen_death_position, off_screen_death_position));
    }
}
