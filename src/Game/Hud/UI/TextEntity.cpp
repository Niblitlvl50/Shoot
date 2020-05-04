
#include "TextEntity.h"
#include "Rendering/IRenderer.h"

using namespace game;

TextEntity::TextEntity(const std::string& text, FontId font, bool centered)
    : m_text(text)
    , m_font(font)
    , m_centered(centered)
{ }

void TextEntity::Draw(mono::IRenderer& renderer) const
{
    renderer.DrawText(m_font, m_text.c_str(), math::Vector(0.1f, -0.05f), m_centered, m_shadow_color);
    renderer.DrawText(m_font, m_text.c_str(), math::ZeroVec, m_centered, m_text_color);
}

void TextEntity::Update(const mono::UpdateContext& update_context)
{

}
