
#include "WeaponStatusElement.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"
#include "AIKnowledge.h"
#include "FontIds.h"

#include "Rendering/Sprite/ISprite.h"
#include "Rendering/Sprite/SpriteFactory.h"

#include "UIElements.h"

#include <cstdio>

using namespace game;

WeaponStatusElement::WeaponStatusElement(const PlayerInfo& player_info, const math::Vector& position)
    : m_player_info(player_info)
{
    m_position = position;
    m_ammo_text = std::make_shared<UITextElement>(FontId::PIXELETTE_SMALL, "0", mono::Color::RGBA(1.0f, 0.0f, 1.0f));
    m_ammo_text->SetPosition(math::Vector(10.0f, -2.0f));
    m_ammo_text->SetScale(math::Vector(8.0f, 8.0f));

    const std::vector<std::string> sprite_files = {
        "res/sprites/bolter.sprite",
        "res/sprites/flak_cannon.sprite",
        "res/sprites/rocket_launcher.sprite"
    };
    m_weapon_sprites = std::make_shared<UISpriteElement>(sprite_files);
    m_weapon_sprites->SetPosition(math::ZeroVec);
    m_weapon_sprites->SetScale(math::Vector(15.0f, 10.0f));

    AddChild(m_ammo_text);
    AddChild(m_weapon_sprites);


    m_frame_sprite = mono::CreateSprite("res/sprites/frame.sprite");
    m_bullet_sprite = mono::CreateSprite("res/sprites/ammo_machinegun.sprite");
}

void WeaponStatusElement::Draw(mono::IRenderer& renderer) const
{
    if(!m_player_info.is_active)
        return;

    //renderer.DrawSprite(*m_bullet_sprite, math::Vector(1.5f, 0.0f));
}

void WeaponStatusElement::Update(unsigned int delta)
{
    char text[32] = { '\0' };
    std::snprintf(text, 32, "%2u", m_player_info.ammunition_left);

    m_ammo_text->SetText(text);
    m_weapon_sprites->SetActiveSprite((size_t)m_player_info.weapon_type);
}
