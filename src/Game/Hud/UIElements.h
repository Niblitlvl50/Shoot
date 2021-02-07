
#pragma once

#include "MonoFwd.h"
#include "Rendering/RenderFwd.h"

#include "IUpdatable.h"
#include "Math/Quad.h"
#include "Math/Matrix.h"
#include "Rendering/Color.h"
#include "Rendering/IDrawable.h"
#include "Rendering/Sprite/ISpriteFactory.h"

#include <string>
#include <vector>
#include <memory>

namespace game
{
    class UIElement
    {
    public:

        virtual ~UIElement() = default;

        virtual void Update(const mono::UpdateContext& context)
        { }
        virtual void Draw(mono::IRenderer& renderer) const
        { }

        void SetPosition(const math::Vector& position);
        void SetScale(const math::Vector& scale);
        void SetRotation(float radians);
        math::Matrix Transform() const;

    protected:

        math::Vector m_position;
        math::Vector m_scale{1.0f, 1.0f};
        float m_rotation = 0.0f;
    };

    class UIOverlay : public mono::IUpdatable, public mono::IDrawable
    {
    public:

        UIOverlay(float width, float height);

        void Update(const mono::UpdateContext& context) override;
        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

        void AddChild(UIElement* element);
        void RemoveChild(UIElement* element);

        math::Matrix Transform() const;

    protected:

        math::Matrix m_projection;
        math::Vector m_position;
        math::Vector m_scale;
        float m_rotation;
        std::vector<UIElement*> m_ui_elements;
    };

    class UITextElement : public UIElement
    {
    public:

        UITextElement(int font_id, const std::string& text, bool centered, const mono::Color::RGBA& color);

        void SetFontId(int new_font_id);
        void SetText(const std::string& new_text);
        void SetColor(const mono::Color::RGBA& new_color);

        void Draw(mono::IRenderer& renderer) const override;

    private:

        int m_font_id;
        std::string m_text;
        bool m_centered;
        mono::Color::RGBA m_color;
    };

    class UISpriteElement : public UIElement
    {
    public:
    
        UISpriteElement(const std::string& sprite_file);
        UISpriteElement(const std::vector<std::string>& sprite_files);

        void SetActiveSprite(size_t index);
        mono::ISprite* GetSprite(size_t index);

        void Update(const mono::UpdateContext& context) override;
        void Draw(mono::IRenderer& renderer) const override;

    private:

        std::vector<mono::ISpritePtr> m_sprites;
        std::vector<mono::SpriteDrawBuffers> m_sprite_buffers;
        std::unique_ptr<mono::IElementBuffer> m_indices;
        size_t m_active_sprite;
    };

    class UISquareElement : public UIElement
    {
    public:
      
        UISquareElement(float width, float height, const mono::Color::RGBA& color);
        UISquareElement(
            float width,
            float height,
            const mono::Color::RGBA& color,
            const mono::Color::RGBA& border_color,
            float border_width);

        void Draw(mono::IRenderer& renderer) const override;

    private:

        std::unique_ptr<mono::IRenderBuffer> m_vertices;
        std::unique_ptr<mono::IRenderBuffer> m_colors;
        std::unique_ptr<mono::IRenderBuffer> m_border_colors;
        std::unique_ptr<mono::IElementBuffer> m_indices;
        const float m_border_width;
    };
}
