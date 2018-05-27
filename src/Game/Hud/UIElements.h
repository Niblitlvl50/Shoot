
#pragma once

#include "Entity/EntityBase.h"
#include "Rendering/Color.h"
#include "Rendering/RenderPtrFwd.h"

#include <string>

namespace game
{
    class UITextElement : public mono::EntityBase
    {
    public:

        UITextElement(int font_id, const std::string& text, const mono::Color::RGBA& color);
        void SetFontId(int new_font_id);
        void SetText(const std::string& new_text);
        void SetColor(const mono::Color::RGBA& new_color);

    private:

        void Draw(mono::IRenderer& renderer) const;
        void Update(unsigned int delta);

        int m_font_id;
        std::string m_text;
        mono::Color::RGBA m_color;
    };

    class UISpriteElement : public mono::EntityBase
    {
    public:
    
        UISpriteElement(const std::vector<std::string>& sprite_files);

        void SetActiveSprite(size_t index);
        mono::ISpritePtr GetSprite(size_t index);

    private:

        void Draw(mono::IRenderer& renderer) const;
        void Update(unsigned int delta);

        size_t m_active_sprite;
        std::vector<mono::ISpritePtr> m_sprites;
    };
}
