
#include "PlayerUIElement.h"
#include "Player/PlayerInfo.h"
#include "FontIds.h"
#include "UIElements.h"
#include "Resources.h"
#include "Weapons/WeaponSystem.h"

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
#include "System/Hash.h"
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
            game::WeaponSystem* weapon_system,
            mono::SpriteSystem* sprite_system)
            : m_player_info(player_info)
            , m_weapon_system(weapon_system)
            , m_sprite_system(sprite_system)
            , m_timer(0.0f)
        {
            m_position = m_offscreen_position = offscreen_position;
            m_screen_position = onscreen_position;

            m_mugshot_hud = new UISpriteElement();
            m_mugshot_hud->SetPosition(-0.6f, 0.0f);

            std::vector<std::string> weapon_sprites;

            const std::vector<WeaponBulletCombination> weapon_combinations = m_weapon_system->GetAllWeaponCombinations();
            for(uint32_t index = 0; index < weapon_combinations.size(); ++index)
            {
                const WeaponBulletCombination& weapon_combination = weapon_combinations[index];
                weapon_sprites.push_back(weapon_combination.sprite_file);

                const uint32_t weapon_hash = hash::Hash(weapon_combination.weapon.c_str());
                m_weapon_hash_to_index[weapon_hash] = index;
            }

            m_weapon_sprites = new UISpriteElement(weapon_sprites);
            m_weapon_sprites->SetPosition(0.15f, 0.0f);

            m_ammo_text = new UITextElement(
                FontId::RUSSOONE_TINY, "", mono::FontCentering::HORIZONTAL_VERTICAL, mono::Color::MAGENTA);
            m_ammo_text->SetPosition(0.55f, 0.1f);
            m_ammo_text->SetScale(0.5f);

            m_chips_text = new UITextElement(
                FontId::RUSSOONE_TINY, "", mono::FontCentering::HORIZONTAL_VERTICAL, mono::Color::GOLDEN_YELLOW);
            m_chips_text->SetPosition(0.55f, -0.1f);
            m_chips_text->SetScale(0.5f);

            m_rubble_text = new UITextElement(
                FontId::RUSSOONE_TINY, "", mono::FontCentering::HORIZONTAL_VERTICAL, mono::Color::GOLDEN_YELLOW);
            m_rubble_text->SetPosition(0.55f, -0.3f);
            m_rubble_text->SetScale(0.5f);

            constexpr mono::Color::RGBA healthbar_red = mono::Color::RGBA(1.0f, 0.3f, 0.3f, 1.0f);
            m_healthbar = new UIBarElement(1.0f, 0.05f, mono::Color::GRAY, 1.0f, 0.05f, healthbar_red);
            m_healthbar->SetPosition(-0.15f, -0.2f);

            m_expbar = new UIBarElement(1.0f, 0.05f, mono::Color::GRAY, 1.0f, 0.05f, mono::Color::GREEN_VIVID);
            m_expbar->SetPosition(-0.15f, -0.35f);

            UISpriteElement* background_hud = new UISpriteElement("res/sprites/player_background_hud.sprite");
            background_hud->AddChild(m_mugshot_hud);
            background_hud->AddChild(m_weapon_sprites);
            background_hud->AddChild(m_ammo_text);
            background_hud->AddChild(m_chips_text);
            background_hud->AddChild(m_rubble_text);
            background_hud->AddChild(m_healthbar);
            background_hud->AddChild(m_expbar);

            AddChild(background_hud);

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

            char chips_text[32] = { '\0' };
            std::snprintf(chips_text, std::size(chips_text), "%d", m_player_info.persistent_data.chips);
            m_chips_text->SetText(chips_text);

            char rubble_text[32] = { '\0' };
            std::snprintf(rubble_text, std::size(rubble_text), "%d", m_player_info.persistent_data.rubble);
            m_rubble_text->SetText(rubble_text);

            const uint32_t weapon_index = m_weapon_hash_to_index[m_player_info.weapon_type.weapon_hash];
            m_weapon_sprites->SetActiveSprite(weapon_index, 0);

            m_healthbar->SetFraction(m_player_info.health_fraction);
            m_expbar->SetFraction(m_player_info.experience_fraction);

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
        game::WeaponSystem* m_weapon_system;
        mono::SpriteSystem* m_sprite_system;

        math::Vector m_screen_position;
        math::Vector m_offscreen_position;
        float m_timer;

        std::unordered_map<uint32_t, uint32_t> m_weapon_hash_to_index;
        class UITextElement* m_ammo_text;
        class UITextElement* m_chips_text;
        class UITextElement* m_rubble_text;
        class UISpriteElement* m_mugshot_hud;
        class UISpriteElement* m_weapon_sprites;
        class UIBarElement* m_healthbar;
        class UIBarElement* m_expbar;

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
            const math::Vector& onscreen_position,
            const math::Vector& offscreen_position)
            : m_player_info(player_info)
            , m_timer(0)
        {
            m_position = m_offscreen_position = offscreen_position;
            m_screen_position = onscreen_position;

            UISquareElement* background = new UISquareElement(
                g_player_death_element_width, g_player_element_height, mono::Color::RGBA(0.0f, 0.0f, 0.0f, 0.8f), mono::Color::BLACK, 1.0f);

            UISpriteElement* skull_sprite = new UISpriteElement("res/sprites/skull_red_eyes.sprite");
            skull_sprite->SetPosition(0.4f, 0.5f);

            UITextElement* death_text = new UITextElement(
                FontId::RUSSOONE_TINY, "You are dead!", mono::FontCentering::HORIZONTAL_VERTICAL, mono::Color::OFF_WHITE);
            death_text->SetPosition(1.75f, 0.60f);
            death_text->SetScale(0.7f);

            UISpriteElement* enter_sprite = new UISpriteElement();
            enter_sprite->SetPosition(1.0f, 0.25f);
            enter_sprite->SetScale(0.3f);

            const std::vector<std::string> continue_sprites = {
                "res/sprites/button_indication.sprite",
                "res/sprites/key_enter.sprite",
            };

            m_continue_sprite = new UISpriteElement(continue_sprites);
            m_continue_sprite->SetPosition(1.0f, 0.25f);
            m_continue_sprite->SetScale(0.75f);

            m_input_type_to_index[mono::InputContextType::Controller] = { 0, 0.75f };
            m_input_type_to_index[mono::InputContextType::Keyboard] = { 1, 0.3f };
            m_input_type_to_index[mono::InputContextType::Mouse] = { 1, 0.3f };

            UITextElement* button_sprite_text = new UITextElement(
                FontId::RUSSOONE_TINY, "to Continue", mono::FontCentering::HORIZONTAL_VERTICAL, mono::Color::GRAY);
            button_sprite_text->SetPosition(1.9f, 0.25f);
            button_sprite_text->SetScale(0.5f);

            AddChild(background);
            AddChild(skull_sprite);
            AddChild(death_text);
            AddChild(m_continue_sprite);
            AddChild(button_sprite_text);
        }

        void Update(const mono::UpdateContext& update_context) override
        {
            UIElement::Update(update_context);
            UpdateAnimation(update_context);
        }

        void UpdateAnimation(const mono::UpdateContext& update_context)
        {
            switch(m_player_info.player_state)
            {
            case game::PlayerState::NOT_SPAWNED:
            case game::PlayerState::ALIVE:
                m_timer -= update_context.delta_s;
                break;
            case game::PlayerState::DEAD:
                m_timer += update_context.delta_s;
                break;
            }

            const auto it = m_input_type_to_index.find(m_player_info.last_used_input);
            if(it != m_input_type_to_index.end())
            {
                m_continue_sprite->SetActiveSprite(it->second.index, 0);
                m_continue_sprite->SetScale(it->second.scale);
            }

            constexpr float transision_duration_s = 0.5f;
            m_timer = std::clamp(m_timer, 0.0f, transision_duration_s);
            AnimatePlayerElement(m_timer, transision_duration_s);
        }

        void AnimatePlayerElement(float time_s, float duration_s)
        {
            m_position.x = math::EaseInCubic(
                time_s, duration_s, m_offscreen_position.x, m_screen_position.x - m_offscreen_position.x);
            m_position.y = math::EaseInCubic(
                time_s, duration_s, m_offscreen_position.y, m_screen_position.y - m_offscreen_position.y);
        }

        const PlayerInfo& m_player_info;

        float m_timer;
        math::Vector m_screen_position;
        math::Vector m_offscreen_position;

        UISpriteElement* m_continue_sprite;

        struct SpriteIndexAndScale
        {
            int index;
            float scale;
        };
        std::unordered_map<mono::InputContextType, SpriteIndexAndScale> m_input_type_to_index;
   };


    class PlayerCoopPowerupElement : public game::UIElement
    {
    public:

        PlayerCoopPowerupElement(
            const PlayerInfo* player_infos, int num_players, float x, float y)
        {
            m_position = math::Vector(x, y);

            m_powerup_bar = new UIBarElement(
                2.0f, 0.1f, mono::Color::GRAY, 2.0f, 0.1f, mono::Color::RED);

            m_powerup_text = new UITextElement(
                FontId::RUSSOONE_TINY, "", mono::FontCentering::DEFAULT_CENTER, mono::Color::OFF_WHITE);
            m_powerup_text->SetScale(0.5f);
            m_powerup_text->SetPosition(0.0f, -0.2f);

            AddChild(m_powerup_bar);
            AddChild(m_powerup_text);
        }

        void Update(const mono::UpdateContext& context) override
        {
            UIElement::Update(context);

            const bool powerup_enabled = (g_coop_powerup.state != CoopPowerUpState::DISABLED);
            m_show = powerup_enabled;

            const float powerup_value = game::g_coop_powerup.powerup_value;
            m_powerup_bar->SetFraction(powerup_value);

            const char* state_string = CoopPowerUpStateToString(g_coop_powerup.state);

            char buffer[256] = { 0 };
            std::snprintf(buffer, std::size(buffer), "Powerup %.2f, %s", powerup_value, state_string);
            m_powerup_text->SetText(buffer);
        }

        UIBarElement* m_powerup_bar;
        UITextElement* m_powerup_text;
    };
}

using namespace game;

PlayerUIElement::PlayerUIElement(
    const PlayerInfo* player_infos,
    int num_players,
    game::WeaponSystem* weapon_system,
    mono::SpriteSystem* sprite_system)
    : UIOverlay(12.0f, 12.0f / mono::RenderSystem::GetWindowAspect())
{
    const float position_x = m_width - g_player_element_half_width;

    const math::Vector hud_positions[] = {
        { g_player_element_half_width, 0.5f },
        { position_x, 0.5f },
        { m_width / 2.0f, 0.5f },
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
        AddChild(new PlayerElement(player_info, on_screen_position, off_screen_position, weapon_system, sprite_system));

        const math::Vector on_screen_death_position = on_screen_position + death_element_offset[index];
        const math::Vector off_screen_death_position = on_screen_death_position + death_element_offscreen_delta[index];
        AddChild(new PlayerDeathElement(player_info, on_screen_death_position, off_screen_death_position));
    }

    AddChild(new PlayerCoopPowerupElement(player_infos, num_players, m_width - 2.5f, m_height - 0.3f));
}
