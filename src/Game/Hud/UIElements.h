
#pragma once

#include "MonoFwd.h"
#include "Rendering/RenderFwd.h"
#include "Math/Quad.h"
#include "Rendering/Color.h"
#include "Rendering/Sprite/ISpriteFactory.h"

#include "IUpdatable.h"

#include <string>
#include <vector>

namespace game
{
    class UITextElement //: public mono::EntityBase
    {
    public:

        UITextElement(int font_id, const std::string& text, bool centered, const mono::Color::RGBA& color);
        void SetFontId(int new_font_id);
        void SetText(const std::string& new_text);
        void SetColor(const mono::Color::RGBA& new_color);

    private:

        void EntityDraw(mono::IRenderer& renderer) const;
        void EntityUpdate(const mono::UpdateContext& update_context);

        int m_font_id;
        std::string m_text;
        bool m_centered;
        mono::Color::RGBA m_color;
    };

    class UISpriteElement //: public mono::EntityBase
    {
    public:
    
        UISpriteElement(
            const std::vector<std::string>& sprite_files,
            mono::SpriteSystem* sprite_system,
            mono::IEntityManager* entity_manager);
        ~UISpriteElement();

        void SetActiveSprite(size_t index);
        mono::ISprite* GetSprite(size_t index);

    private:

        void EntityDraw(mono::IRenderer& renderer) const;
        void EntityUpdate(const mono::UpdateContext& update_context);

        mono::SpriteSystem* m_sprite_system;
        mono::IEntityManager* m_entity_manager;
        std::vector<uint32_t> m_sprite_entities;
        size_t m_active_sprite;
    };

    class UISquareElement //: public mono::EntityBase
    {
    public:
      
        UISquareElement(const math::Quad& square, const mono::Color::RGBA& color);
        UISquareElement(const math::Quad& square, const mono::Color::RGBA& color, const mono::Color::RGBA& border_color, float border_width);

    private:

        void EntityDraw(mono::IRenderer& renderer) const;
        void EntityUpdate(const mono::UpdateContext& update_context);

        const math::Quad m_square;
        const mono::Color::RGBA m_color;
        const mono::Color::RGBA m_border_color;
        const float m_border_width;
    };
}
