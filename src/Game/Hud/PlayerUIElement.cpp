
#include "PlayerUIElement.h"

#include "Player/PlayerInfo.h"
#include "FontIds.h"
#include "UIElements.h"
#include "Math/EasingFunctions.h"

#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"
#include "Rendering/Sprite/ISprite.h"
#include "Rendering/Sprite/SpriteFactory.h"

#include <cstdio>

namespace game
{
    class ReloadLine : public game::UIElement
    {
    public:

        ReloadLine(const game::PlayerInfo& player_info)
            : m_player_info(player_info)
        {}

        void Draw(mono::IRenderer& renderer) const
        {
            const bool is_reloading = (m_player_info.weapon_state == game::WeaponState::RELOADING);
            if(is_reloading)
            {
                const std::vector<math::Vector> reload_line = {
                    math::Vector(0.0f, 0.0f),
                    math::Vector(50.0f, 0.0f)
                };

                const float reload_dot =
                    math::Length(reload_line.back() - reload_line.front()) * float(m_player_info.weapon_reload_percentage) / 100.f;

                renderer.DrawLines(reload_line, mono::Color::RED, 4.0f);
                renderer.DrawPoints({ math::Vector(reload_dot, 0.0f) }, mono::Color::BLACK, 8.0f);
            }
        }

        const game::PlayerInfo& m_player_info;
    };

    class HeartContainer : public game::UIElement
    {
    public:

        static constexpr const int max_lives = 3;
        static constexpr const char* heart_sprite = "res/sprites/heart.sprite";

        HeartContainer()
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

        void SetLives(int lives)
        {

            for(int index = 0; index < max_lives; ++index)
            {
                const bool alive = index < lives;
                const mono::Color::RGBA shade = alive ? mono::Color::WHITE : mono::Color::GRAY;
                const int animation = alive ? m_deafault_id : m_dead_id;

                mono::ISprite* sprite = m_hearts[index]->GetSprite(0);
                sprite->SetShade(shade);

                if(animation != sprite->GetActiveAnimation())
                    sprite->SetAnimation(animation);
            }
        }

        game::UISpriteElement* m_hearts[max_lives];
        int m_deafault_id;
        int m_dead_id;
    };
}

using namespace game;

PlayerUIElement::PlayerUIElement(const PlayerInfo& player_info)
    : UIOverlay(400.0f, 400.0f)
    , m_player_info(player_info)
    , m_timer(0)
    , m_current_score(m_player_info.score)
{
    m_position = m_offscreen_position = math::Vector(-200.0f, 0.0f);
    m_screen_position = math::Vector(0.0f, 0.0f);

    m_background = new UISquareElement(70.0f, 18.0f, mono::Color::OFF_WHITE, mono::Color::GRAY, 1.0f);
    m_background->SetPosition(math::Vector(2.0f, 2.0f));

    m_mugshot_sprite = new UISpriteElement("res/sprites/doomguy.sprite");
    m_mugshot_sprite->SetPosition(math::Vector(10.0f, 10.0f));
    m_mugshot_sprite->SetScale(math::Vector(8.0f, 8.0f));
    m_mugshot_sprite->GetSprite(0)->SetAnimation(1);

    const std::vector<std::string> sprite_files = {
        "res/sprites/bolter.sprite",
        "res/sprites/flak_cannon.sprite",
        "res/sprites/rocket_launcher.sprite",
        "res/sprites/bolter.sprite",
        "res/sprites/bolter.sprite",
        "res/sprites/bolter.sprite",
    };
    m_weapon_sprites = new UISpriteElement(sprite_files);
    m_weapon_sprites->SetPosition(math::Vector(26.0f, 10.0f));
    m_weapon_sprites->SetScale(math::Vector(14.0f, 14.0f));

    m_ammo_text = new UITextElement(shared::FontId::PIXELETTE_LARGE, "0", mono::FontCentering::HORIZONTAL_VERTICAL, mono::Color::MAGENTA);
    m_ammo_text->SetPosition(math::Vector(42.0f, 8.5f));
    m_ammo_text->SetScale(math::Vector(2.0f, 2.0f));

    m_weapon_state_text = new UITextElement(shared::FontId::PIXELETTE_LARGE, "", mono::FontCentering::HORIZONTAL_VERTICAL, mono::Color::MAGENTA);
    m_weapon_state_text->SetPosition(math::Vector(50.0f, 8.5f));
    m_weapon_state_text->SetScale(math::Vector(2.0f, 2.0f));

    ReloadLine* weapon_reload_line = new ReloadLine(m_player_info);
    weapon_reload_line->SetPosition(math::Vector(10.0f, 25.0f));

    m_hearts = new HeartContainer();
    m_hearts->SetPosition(math::Vector(20.0f, 380.0f));
    m_hearts->SetScale(math::Vector(40.0f, 40.0f));

    m_score_text = new UITextElement(shared::FontId::PIXELETTE_SMALL, "", mono::FontCentering::HORIZONTAL_VERTICAL, mono::Color::MAGENTA);
    m_score_text->SetPosition(math::Vector(200.0f, 10.0f));
    m_score_text->SetScale(math::Vector(10.0f, 10.0f));

    AddChild(m_background);
    AddChild(m_mugshot_sprite);
    AddChild(m_weapon_sprites);
    AddChild(m_ammo_text);
    AddChild(m_weapon_state_text);
    AddChild(weapon_reload_line);
    AddChild(m_hearts);
    AddChild(m_score_text);
}

void PlayerUIElement::Update(const mono::UpdateContext& update_context)
{
    UIOverlay::Update(update_context);

    if(m_player_info.score != m_current_score)
    {
        //const int score_diff = m_player_info.score - m_current_score;
        m_current_score += 10;
        m_current_score = std::min(m_player_info.score, m_current_score);
    }

    char score_buffer[256] = { 0 };
    std::snprintf(score_buffer, std::size(score_buffer), "score %010d", m_current_score);
    m_score_text->SetText(score_buffer);

    m_hearts->SetLives(m_player_info.lives);

    char ammo_text[32] = { '\0' };
    std::snprintf(ammo_text, std::size(ammo_text), "%2u", m_player_info.magazine_left);

    m_ammo_text->SetText(ammo_text);
    m_weapon_sprites->SetActiveSprite((size_t)m_player_info.weapon_type);
    m_weapon_state_text->SetText(WeaponStateToString(m_player_info.weapon_state));

    if(m_player_info.player_state != game::PlayerState::NOT_SPAWNED && m_timer < 1.0f)
    {
        m_position.x = math::EaseOutCubic(m_timer, 1.0f, m_offscreen_position.x, m_screen_position.x - m_offscreen_position.x);
        m_timer += float(update_context.delta_ms) / 1000.0f;
    }
    else if(m_player_info.player_state == game::PlayerState::NOT_SPAWNED && m_timer > 0.0f)
    {
        m_position.x = math::EaseInCubic(m_timer, 1.0f, m_offscreen_position.x, m_screen_position.x - m_offscreen_position.x);
        m_timer -= float(update_context.delta_ms) / 1000.0f;
    }
}
