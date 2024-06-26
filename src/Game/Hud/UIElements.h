
#pragma once

#include "MonoFwd.h"
#include "Rendering/RenderFwd.h"

#include "IUpdatable.h"
#include "Math/Quad.h"
#include "Math/Matrix.h"
#include "Rendering/Color.h"
#include "Rendering/IDrawable.h"
#include "Rendering/Sprite/ISpriteFactory.h"
#include "Rendering/Text/TextFlags.h"
#include "Rendering/Texture/ITextureFactory.h"
#include "Rendering/Texture/TextureBufferFactory.h"
#include "Rendering/Text/TextBufferFactory.h"

#include <string>
#include <vector>
#include <memory>

namespace game
{
    class UIElement : public mono::IUpdatable, public mono::IDrawable
    {
    public:

        UIElement();
        virtual ~UIElement();

        void Update(const mono::UpdateContext& context) override;
        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

        virtual void Show();
        virtual void Hide();

        void SetPosition(const math::Vector& position);
        void SetPosition(float x, float y);
        const math::Vector& GetPosition() const;
        void SetScale(const math::Vector& scale);
        void SetScale(float uniform_scale);
        void SetRotation(float radians);
        math::Matrix LocalTransform() const;
        math::Matrix Transform() const;

        void AddChild(UIElement* element);
        void RemoveChild(UIElement* element);

    protected:

        UIElement* m_parent;
        math::Vector m_position;
        math::Vector m_scale;
        float m_rotation;
        std::vector<UIElement*> m_ui_elements;
        bool m_show;
    };

    class UIOverlay : public UIElement
    {
    public:

        UIOverlay(float width, float height);
        void Draw(mono::IRenderer& renderer) const override;

    protected:

        const float m_width;
        const float m_height;
        const math::Matrix m_projection;
    };

    class UITextElement : public UIElement
    {
    public:

        UITextElement(int font_id, const std::string& text, mono::FontCentering centering, const mono::Color::RGBA& color);
        void SetText(const std::string& new_text);
        void SetColor(const mono::Color::RGBA& new_color);
        void SetAlpha(float alpha);

        math::Quad GetBounds() const;

    private:

        void Draw(mono::IRenderer& renderer) const override;

        int m_font_id;
        std::string m_text;
        mono::FontCentering m_centering;
        mono::Color::RGBA m_color;

        mono::TextDrawBuffers m_draw_buffers;
    };

    class UISpriteElement : public UIElement
    {
    public:
    
        UISpriteElement();
        UISpriteElement(const std::string& sprite_file);
        UISpriteElement(const std::vector<std::string>& sprite_files);

        void SetSprite(const std::string& sprite_file);
        void SetSprites(const std::vector<std::string>& sprite_files);

        void SetActiveSprite(uint32_t index, uint32_t animation_id);
        mono::ISprite* GetSprite(uint32_t index);

        void Update(const mono::UpdateContext& context) override;
        void Draw(mono::IRenderer& renderer) const override;

    private:

        std::vector<mono::ISpritePtr> m_sprites;
        std::vector<mono::SpriteDrawBuffers> m_sprite_buffers;
        std::unique_ptr<mono::IElementBuffer> m_indices;
        uint32_t m_active_sprite;
    };

    class UITextureElement : public UIElement
    {
    public:

        UITextureElement();
        UITextureElement(const char* texture);
        void SetTexture(const char* texture);
        void Draw(mono::IRenderer& renderer) const override;

    private:

        mono::TextureDrawBuffers m_draw_buffers;
        mono::ITexturePtr m_texture;
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
        ~UISquareElement();

        void Draw(mono::IRenderer& renderer) const override;

        void SetColor(const mono::Color::RGBA& color);
        const mono::Color::RGBA& GetColor() const;

        void SetBorderColor(const mono::Color::RGBA& color);
        const mono::Color::RGBA& GetBorderColor() const;

    private:

        std::unique_ptr<mono::IRenderBuffer> m_vertices;
        std::unique_ptr<mono::IRenderBuffer> m_colors;
        std::unique_ptr<mono::IRenderBuffer> m_border_colors;
        std::unique_ptr<mono::IElementBuffer> m_indices;

        const float m_border_width;
        mono::Color::RGBA m_color;
        mono::Color::RGBA m_border_color;
    };

    class UIBarElement : public UIElement
    {
    public:

        enum class Direction
        {
            HORIZONTAL,
            VERTICAL
        };

        UIBarElement(
            float background_width, float background_height, const mono::Color::RGBA& background_color,
            float foreground_width, float foreground_height, const mono::Color::RGBA& foreground_color
        );

        void SetFraction(float fraction);
        void SetDirection(Direction new_direction);

    private:

        void Update(const mono::UpdateContext& context) override;
        void Draw(mono::IRenderer& renderer) const override;

        float m_fraction;
        float m_target_fraction;
        float m_velocity;
        Direction m_direction;

        std::unique_ptr<mono::IRenderBuffer> m_vertices;
        std::unique_ptr<mono::IRenderBuffer> m_background_colors;
        std::unique_ptr<mono::IRenderBuffer> m_foreground_colors;
        std::unique_ptr<mono::IElementBuffer> m_indices;
    };
}
