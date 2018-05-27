
#include "UIElements.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Sprite/ISprite.h"
#include "Rendering/Sprite/SpriteFactory.h"

using namespace game;

UITextElement::UITextElement(int font_id, const std::string& text, const mono::Color::RGBA& color)
    : m_font_id(font_id)
    , m_text(text)
    , m_color(color)
{ }

void UITextElement::SetFontId(int new_font_id)
{
    m_font_id = new_font_id;
}

void UITextElement::SetText(const std::string& new_text)
{
    m_text = new_text;
}

void UITextElement::SetColor(const mono::Color::RGBA& new_color)
{
    m_color = new_color;
}

void UITextElement::Draw(mono::IRenderer& renderer) const
{
    renderer.DrawText(m_font_id, m_text.c_str(), math::ZeroVec, false, m_color);
}

void UITextElement::Update(unsigned int delta)
{ }


UISpriteElement::UISpriteElement(const std::vector<std::string>& sprite_files)
    : m_active_sprite(0)
{
    for(const std::string& sprite_file : sprite_files)
        m_sprites.push_back(mono::CreateSprite(sprite_file.c_str()));
}

void UISpriteElement::SetActiveSprite(size_t index)
{
    m_active_sprite = index;
}

mono::ISpritePtr UISpriteElement::GetSprite(size_t index)
{
    return m_sprites[index];
}

void UISpriteElement::Draw(mono::IRenderer& renderer) const
{
    renderer.DrawSprite(*m_sprites[m_active_sprite]);
}

void UISpriteElement::Update(unsigned int delta)
{
    for(auto& sprite : m_sprites)
        sprite->doUpdate(delta);
}
