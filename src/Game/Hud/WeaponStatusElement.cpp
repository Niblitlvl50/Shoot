
#include "WeaponStatusElement.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"
#include "AIKnowledge.h"
#include "FontIds.h"

#include "Rendering/Sprite/ISprite.h"
#include "Rendering/Sprite/SpriteFactory.h"

#include "UIElements.h"

#include "Math/EasingFunctions.h"

#include <cstdio>

using namespace game;

WeaponStatusElement::WeaponStatusElement(
    const PlayerInfo& player_info, const math::Vector& position, const math::Vector& offscreen_position)
    : m_player_info(player_info)
    , m_screen_position(position)
    , m_offscreen_position(offscreen_position)
    , m_timer(0)
{
    m_position = offscreen_position;

    m_ammo_text = std::make_unique<UITextElement>(shared::FontId::PIXELETTE_SMALL, "0", false, mono::Color::MAGENTA);
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

    AddChild(m_frame_sprite.get());
    AddChild(m_weapon_sprites.get());
    AddChild(m_ammo_text.get());
}

WeaponStatusElement::~WeaponStatusElement()
{
    RemoveChild(m_frame_sprite.get());
    RemoveChild(m_weapon_sprites.get());
    RemoveChild(m_ammo_text.get());
}

void WeaponStatusElement::Draw(mono::IRenderer& renderer) const
{ }

void WeaponStatusElement::Update(const mono::UpdateContext& update_context)
{
    char text[32] = { '\0' };
    std::snprintf(text, 32, "%2u / %3u", m_player_info.magazine_left, m_player_info.ammunition_left);

    m_ammo_text->SetText(text);
    m_weapon_sprites->SetActiveSprite((size_t)m_player_info.weapon_type);

    if(m_player_info.player_state == game::PlayerState::ALIVE && m_timer < 1.0f)
    {
        m_position.x = math::EaseOutCubic(m_timer, 1.0f, m_offscreen_position.x, m_screen_position.x - m_offscreen_position.x);
        m_timer += float(update_context.delta_ms) / 1000.0f;
    }
    else if(m_player_info.player_state != game::PlayerState::ALIVE && m_timer > 0.0f)
    {
        m_position.x = math::EaseInCubic(m_timer, 1.0f, m_offscreen_position.x, m_screen_position.x - m_offscreen_position.x);
        m_timer -= float(update_context.delta_ms) / 1000.0f;
    }
}
