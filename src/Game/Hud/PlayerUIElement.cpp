
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

    UISquareElement* background = new UISquareElement(5.5f, 1.0f, mono::Color::RGBA(0.2f, 0.2f, 0.2f, 0.5f), mono::Color::BLACK, 1.0f);

    HeartContainer* hearts = new HeartContainer(player_info);
    hearts->SetPosition(0.5f, 0.5f);

    const std::vector<std::string> sprite_files = {
        "res/sprites/bolter.sprite",
        "res/sprites/flak_cannon.sprite",
        "res/sprites/rocket_launcher.sprite",
        "res/sprites/bolter.sprite",
        "res/sprites/flak_cannon.sprite",
        "res/sprites/bolter.sprite",
    };
    m_weapon_sprites = new UISpriteElement(sprite_files);
    m_weapon_sprites->SetPosition(3.5f, 0.5f);

    m_ammo_text = new UITextElement(shared::FontId::PIXELETTE_TINY, "", mono::FontCentering::HORIZONTAL_VERTICAL, mono::Color::MAGENTA);
    m_ammo_text->SetPosition(4.5f, 0.5f);

    AddChild(background);
    AddChild(hearts);
    AddChild(m_weapon_sprites);
    AddChild(m_ammo_text);
}

void PlayerUIElement::Update(const mono::UpdateContext& update_context)
{
    UIOverlay::Update(update_context);

    char ammo_text[32] = { '\0' };
    std::snprintf(ammo_text, std::size(ammo_text), "%2u", m_player_info.magazine_left);
    m_ammo_text->SetText(ammo_text);

    //m_weapon_sprites->SetActiveSprite((size_t)m_player_info.weapon_type);

    if(m_player_info.player_state != game::PlayerState::NOT_SPAWNED && m_timer < 1.0f)
    {
        m_position.x = math::EaseOutCubic(m_timer, 1.0f, m_offscreen_position.x, m_screen_position.x - m_offscreen_position.x);
        m_position.y = math::EaseOutCubic(m_timer, 1.0f, m_offscreen_position.y, m_screen_position.y - m_offscreen_position.y);
        m_timer += update_context.delta_s;
    }
    else if(m_player_info.player_state == game::PlayerState::NOT_SPAWNED && m_timer > 0.0f)
    {
        m_position.x = math::EaseInCubic(m_timer, 1.0f, m_offscreen_position.x, m_screen_position.x - m_offscreen_position.x);
        m_position.y = math::EaseInCubic(m_timer, 1.0f, m_offscreen_position.y, m_screen_position.y - m_offscreen_position.y);
        m_timer -= update_context.delta_s;
    }

    m_timer = std::clamp(m_timer, 0.0f, 1.0f);
}
