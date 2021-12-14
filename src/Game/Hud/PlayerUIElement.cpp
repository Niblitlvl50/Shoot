
#include "PlayerUIElement.h"

#include "Player/PlayerInfo.h"
#include "FontIds.h"
#include "UIElements.h"
#include "Math/EasingFunctions.h"

#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"
#include "Rendering/Sprite/ISprite.h"
#include "Rendering/Sprite/SpriteFactory.h"
#include "Rendering/RenderSystem.h"

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
            UIElement::Draw(renderer);

            const bool is_reloading = (m_player_info.weapon_state == game::WeaponState::RELOADING);
            if(is_reloading)
            {
                const math::Matrix& transform = renderer.GetTransform() * Transform();
                const auto transform_scope = mono::MakeTransformScope(transform, &renderer);

                const std::vector<math::Vector> reload_line = {
                    math::Vector(0.0f, 0.0f),
                    math::Vector(1.0f, 0.0f)
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
}

using namespace game;

PlayerUIElement::PlayerUIElement(const PlayerInfo& player_info)
    : UIOverlay(16.0f, 16.0f / mono::GetWindowAspect())
    , m_player_info(player_info)
    , m_timer(0)
{

    m_position = m_offscreen_position = math::Vector(0.0f, -5.0f);
    m_screen_position = math::Vector(0.0f, 0.0f);

    //UISquareElement* background = new UISquareElement(m_width, 1.0f, mono::Color::RGBA(0.2f, 0.2f, 0.2f, 0.5f), mono::Color::BLACK, 1.0f);

    const std::vector<std::string> sprite_files = {
        "res/sprites/bolter.sprite",
        "res/sprites/flak_cannon.sprite",
        "res/sprites/rocket_launcher.sprite",
        "res/sprites/bolter.sprite",
        "res/sprites/flak_cannon.sprite",
        "res/sprites/bolter.sprite",
    };
    m_weapon_sprites = new UISpriteElement(sprite_files);
    m_weapon_sprites->SetPosition(10.0f, 0.5f);

    m_ammo_text = new UITextElement(shared::FontId::PIXELETTE_TINY, "", mono::FontCentering::HORIZONTAL_VERTICAL, mono::Color::MAGENTA);
    m_ammo_text->SetPosition(3.25f, 0.5f);

    m_weapon_state_text = new UITextElement(shared::FontId::PIXELETTE_TINY, "", mono::FontCentering::HORIZONTAL_VERTICAL, mono::Color::MAGENTA);
    m_weapon_state_text->SetPosition(3.25f, 1.0f);

    ReloadLine* weapon_reload_line = new ReloadLine(m_player_info);
    weapon_reload_line->SetPosition(12.0f, 0.5f);
    weapon_reload_line->SetScale(math::Vector(3.0f, 1.0f));

    HeartContainer* hearts = new HeartContainer(player_info);
    hearts->SetPosition(8.0f, 0.5f); //m_height - 0.5f);

    //AddChild(background);
    AddChild(m_weapon_sprites);
    AddChild(m_ammo_text);
    //AddChild(m_weapon_state_text);
    AddChild(weapon_reload_line);
    AddChild(hearts);
}

void PlayerUIElement::Update(const mono::UpdateContext& update_context)
{
    UIOverlay::Update(update_context);

    char ammo_text[32] = { '\0' };
    std::snprintf(ammo_text, std::size(ammo_text), "%2u", m_player_info.magazine_left);

    m_ammo_text->SetText(ammo_text);
    //m_weapon_sprites->SetActiveSprite((size_t)m_player_info.weapon_type);
    m_weapon_state_text->SetText(WeaponStateToString(m_player_info.weapon_state));

    if(m_player_info.player_state != game::PlayerState::NOT_SPAWNED && m_timer < 1.0f)
    {
        m_position.x = math::EaseOutCubic(m_timer, 1.0f, m_offscreen_position.x, m_screen_position.x - m_offscreen_position.x);
        m_position.y = math::EaseOutCubic(m_timer, 1.0f, m_offscreen_position.y, m_screen_position.y - m_offscreen_position.y);
        m_timer += float(update_context.delta_ms) / 1000.0f;
    }
    else if(m_player_info.player_state == game::PlayerState::NOT_SPAWNED && m_timer > 0.0f)
    {
        m_position.x = math::EaseInCubic(m_timer, 1.0f, m_offscreen_position.x, m_screen_position.x - m_offscreen_position.x);
        m_position.y = math::EaseInCubic(m_timer, 1.0f, m_offscreen_position.y, m_screen_position.y - m_offscreen_position.y);
        m_timer -= float(update_context.delta_ms) / 1000.0f;
    }
}
