
#pragma once

#include "Entity/EntityBase.h"
#include "Math/Quad.h"
#include "Rendering/Color.h"
#include "Rendering/RenderPtrFwd.h"
#include "Rendering/Sprite/ISpriteFactory.h"

#include <string>

namespace game
{
    class UITextElement : public mono::EntityBase
    {
    public:

        UITextElement(int font_id, const std::string& text, bool centered, const mono::Color::RGBA& color);
        void SetFontId(int new_font_id);
        void SetText(const std::string& new_text);
        void SetColor(const mono::Color::RGBA& new_color);

    private:

        void Draw(mono::IRenderer& renderer) const;
        void Update(const mono::UpdateContext& update_context);

        int m_font_id;
        std::string m_text;
        bool m_centered;
        mono::Color::RGBA m_color;
    };

    class UISpriteElement : public mono::EntityBase
    {
    public:
    
        UISpriteElement(const std::vector<std::string>& sprite_files);
        ~UISpriteElement();

        void SetActiveSprite(size_t index);
        const mono::ISpritePtr& GetSprite(size_t index) const;

    private:

        void Draw(mono::IRenderer& renderer) const;
        void Update(const mono::UpdateContext& update_context);

        size_t m_active_sprite;
        std::vector<mono::ISpritePtr> m_sprites;
    };

    class UISquareElement : public mono::EntityBase
    {
    public:
      
        UISquareElement(const math::Quad& square, const mono::Color::RGBA& color);
        UISquareElement(const math::Quad& square, const mono::Color::RGBA& color, const mono::Color::RGBA& border_color, float border_width);

    private:

        void Draw(mono::IRenderer& renderer) const;
        void Update(const mono::UpdateContext& update_context);

        const math::Quad m_square;
        const mono::Color::RGBA m_color;
        const mono::Color::RGBA m_border_color;
        const float m_border_width;
    };
}
