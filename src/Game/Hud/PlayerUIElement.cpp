
#include "PlayerUIElement.h"

#include "AIKnowledge.h"
#include "FontIds.h"
#include "UIElements.h"
#include "Actions/EasingFunctions.h"

#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"
#include "Rendering/Sprite/ISprite.h"
#include "Rendering/Sprite/SpriteFactory.h"

#include <cstdio>

using namespace game;

PlayerUIElement::PlayerUIElement(
    const PlayerInfo& player_info, const math::Vector& position, const math::Vector& offscreen_position)
    : m_player_info(player_info)
    , m_screen_position(position)
    , m_offscreen_position(offscreen_position)
    , m_timer(0)
    , m_current_score(m_player_info.score)
{
    m_position = offscreen_position;

    m_ammo_text = std::make_unique<UITextElement>(FontId::PIXELETTE_SMALL, "0", false, mono::Color::MAGENTA);
    m_ammo_text->SetPosition(math::Vector(11.0f, -1.5f));
    m_ammo_text->SetScale(math::Vector(6.0f, 6.0f));

    const std::vector<std::string> sprite_files = {
        "res/sprites/bolter.sprite",
        "res/sprites/flak_cannon.sprite",
        "res/sprites/rocket_launcher.sprite",
        "res/sprites/bolter.sprite",
        "res/sprites/bolter.sprite"
    };
    m_weapon_sprites = std::make_unique<UISpriteElement>(sprite_files);
    m_weapon_sprites->SetPosition(math::ZeroVec);
    m_weapon_sprites->SetScale(math::Vector(10.0f, 10.0f));

    const std::vector<std::string> frame_sprites = {
        "res/sprites/frame.sprite"
    };

    m_frame_sprite = std::make_unique<UISpriteElement>(frame_sprites);
    m_frame_sprite->SetPosition(math::Vector(14.0f, 0.0f));
    m_frame_sprite->SetScale(math::Vector(15.0f, 10.0f));

    const std::vector<std::string> mugshot_sprites = {
        "res/sprites/doomguy.sprite"
    };
    m_mugshot_sprite = std::make_unique<UISpriteElement>(mugshot_sprites);
    m_mugshot_sprite->SetPosition(math::Vector(10.0f, 10.0f));
    m_mugshot_sprite->SetScale(math::Vector(8.0f, 8.0f));
    m_mugshot_sprite->GetSprite(0)->SetAnimation(1);

    m_score_text = std::make_unique<UITextElement>(FontId::PIXELETTE_SMALL, "", false, mono::Color::BLUE);
    m_score_text->SetPosition(math::Vector(14.0f, 10.0f));

    AddChild(m_frame_sprite.get());
    AddChild(m_weapon_sprites.get());
    AddChild(m_mugshot_sprite.get());
    AddChild(m_ammo_text.get());
    AddChild(m_score_text.get());
}

PlayerUIElement::~PlayerUIElement()
{
    RemoveChild(m_frame_sprite.get());
    RemoveChild(m_weapon_sprites.get());
    RemoveChild(m_mugshot_sprite.get());
    RemoveChild(m_ammo_text.get());
    RemoveChild(m_score_text.get());
}

void PlayerUIElement::Draw(mono::IRenderer& renderer) const
{ }

void PlayerUIElement::Update(const mono::UpdateContext& update_context)
{
    char score_buffer[256] = { 0 };
    std::snprintf(score_buffer, std::size(score_buffer), "score %010d", m_current_score);
    m_score_text->SetText(score_buffer);

    char ammo_text[32] = { '\0' };
    std::snprintf(ammo_text, std::size(ammo_text), "%2u / %3u", m_player_info.magazine_left, m_player_info.ammunition_left);

    m_ammo_text->SetText(ammo_text);
    m_weapon_sprites->SetActiveSprite((size_t)m_player_info.weapon_type);

    if(m_player_info.is_active && m_timer < 1.0f)
    {
        m_position.x = EaseOutCubic(m_timer, 1.0f, m_offscreen_position.x, m_screen_position.x - m_offscreen_position.x);
        m_timer += float(update_context.delta_ms) / 1000.0f;
    }
    else if(!m_player_info.is_active && m_timer > 0.0f)
    {
        m_position.x = EaseInCubic(m_timer, 1.0f, m_offscreen_position.x, m_screen_position.x - m_offscreen_position.x);
        m_timer -= float(update_context.delta_ms) / 1000.0f;
    }
}
