
#pragma once

#include "MonoFwd.h"
#include "Rendering/RenderFwd.h"

#include "IUpdatable.h"
#include "Math/Quad.h"
#include "Math/Matrix.h"
#include "Rendering/Color.h"
#include "Rendering/IDrawable.h"
#include "Rendering/Sprite/ISpriteFactory.h"
#include "Rendering/Sprite/SpriteBufferFactory.h"
#include "Rendering/Text/TextFlags.h"
#include "Rendering/Texture/ITextureFactory.h"
#include "Rendering/Texture/TextureBufferFactory.h"
#include "Rendering/Text/TextBufferFactory.h"

#include <string>
#include <vector>
#include <unordered_map>
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

        virtual math::Quad BoundingBox() const override;
        virtual math::Quad LocalBoundingBox() const;
        virtual void DrawElement(mono::IRenderer& renderer) const { }
        virtual void Show();
        virtual void Hide();

        void SetPosition(const math::Vector& position);
        void SetPosition(float x, float y);
        const math::Vector& GetPosition() const;
        void SetScale(const math::Vector& scale);
        void SetScale(float uniform_scale);
        void SetRotation(float radians);
        void SetAchorPoint(mono::AnchorPoint anchor_point);

        math::Matrix LocalTransform() const;
        math::Matrix Transform() const;
        math::Vector GetAnchorOffset() const;

        void AddChild(UIElement* element);
        void RemoveChild(UIElement* element);
        void DeleteChild(UIElement* element);

    protected:

        UIElement* m_parent;

        math::Vector m_position;
        math::Vector m_scale;
        float m_rotation;
        mono::AnchorPoint m_anchor_point;

        std::vector<UIElement*> m_ui_elements;
        bool m_show;
    };

    class UIOverlay : public UIElement
    {
    public:

        UIOverlay(float width, float height);
        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;
        math::Quad LocalBoundingBox() const override;

    protected:

        const float m_width;
        const float m_height;
        const math::Matrix m_projection;
    };

    class UITextElement : public UIElement
    {
    public:

        UITextElement(int font_id, const std::string& text, const mono::Color::RGBA& color);
        void SetText(const std::string& new_text);
        void SetColor(const mono::Color::RGBA& new_color);
        void SetShadowColor(const mono::Color::RGBA& shadow_color);
        void SetShadowOffset(float x, float y);
        void SetEnableShadow(bool shadow);
        void SetAlpha(float alpha);

        math::Quad LocalBoundingBox() const override;

    private:

        void DrawElement(mono::IRenderer& renderer) const override;

        int m_font_id;
        std::string m_text;
        mono::Color::RGBA m_color;
        mono::Color::RGBA m_shadow_color;
        math::Vector m_shadow_offset;
        bool m_draw_shadow;

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
        void DrawElement(mono::IRenderer& renderer) const override;
        math::Quad LocalBoundingBox() const override;

    private:

        std::vector<mono::ISpritePtr> m_sprites;
        std::vector<math::Quad> m_sprite_bounds;

        mutable std::unordered_map<uint32_t, mono::SpriteDrawBuffers> m_sprite_buffers;
        std::unique_ptr<mono::IElementBuffer> m_indices;

        uint32_t m_active_sprite;
    };

    class UITextureElement : public UIElement
    {
    public:

        UITextureElement();
        UITextureElement(const char* texture, float pixels_per_meter);
        void SetTexture(const char* texture, float pixels_per_meter);
        void DrawElement(mono::IRenderer& renderer) const override;
        math::Quad LocalBoundingBox() const override;

        math::Vector GetTextureSize() const;

    private:

        mono::TextureDrawBuffers m_draw_buffers;
        mono::ITexturePtr m_texture;
        float m_pixels_per_meter;
    };

    class UISpriteBarElement : public UIElement
    {
    public:

        UISpriteBarElement();
        void SetSpacing(float spacing);
        void PushSprite(int sprite_handle, const char* sprite_file);
        void RemoveSprite(int sprite_handle);

    private:
        void RecalculateLayout();

        float m_spacing;
        
        struct UISpriteData
        {
            int handle;
            UISpriteElement* ui_sprite;
        };
        std::vector<UISpriteData> m_sprites;
    };

    class UIActivePowerupsElement : public UIElement
    {
    public:

        UIActivePowerupsElement();
        void SetSpacing(float spacing);
        void PushSprite(int sprite_handle, const char* sprite_file, const char* text);
        void RemoveSprite(int sprite_handle);

    private:
        void RecalculateLayout();

        float m_spacing;
        
        struct UISpriteData
        {
            int handle;
            UISpriteElement* ui_sprite;
            UITextElement* ui_text;
        };
        std::vector<UISpriteData> m_sprites;
    };

    class UITextureBoxElement : public UIElement
    {
    public:

        UITextureBoxElement();
        UITextureBoxElement(const char* left_sprite, const char* mid_sprite, const char* right_sprite, float pixels_per_meter);
        void SetTextures(const char* left_sprite, const char* mid_sprite, const char* right_sprite, float pixels_per_meter);
        void SetWidth(float width);

    private:

        UITextureElement* m_left;
        UITextureElement* m_mid;
        UITextureElement* m_right;

        float m_pixels_per_meter;
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

        void DrawElement(mono::IRenderer& renderer) const override;
        math::Quad LocalBoundingBox() const override;

        void SetColor(const mono::Color::RGBA& color);
        const mono::Color::RGBA& GetColor() const;

        void SetBorderColor(const mono::Color::RGBA& color);
        const mono::Color::RGBA& GetBorderColor() const;

    private:

        std::unique_ptr<mono::IRenderBuffer> m_vertices;
        std::unique_ptr<mono::IRenderBuffer> m_colors;
        std::unique_ptr<mono::IRenderBuffer> m_border_colors;
        std::unique_ptr<mono::IElementBuffer> m_indices;

        const float m_width;
        const float m_height;
        const float m_border_width;
        mono::Color::RGBA m_color;
        mono::Color::RGBA m_border_color;
    };

    class UICircleElement : public UIElement
    {
    public:
      
        UICircleElement(float radii, const mono::Color::RGBA& color);
        ~UICircleElement();

        void DrawElement(mono::IRenderer& renderer) const override;
        math::Quad LocalBoundingBox() const override;

    private:

        float m_radii;
        mono::Color::RGBA m_color;

        std::unique_ptr<mono::IRenderBuffer> m_vertices;
        std::unique_ptr<mono::IRenderBuffer> m_colors;
        std::unique_ptr<mono::IElementBuffer> m_indices;
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
            float background_width,
            float background_height,
            const mono::Color::RGBA& background_color,
            const mono::Color::RGBA& foreground_color);

        void SetFraction(float fraction);
        void SetDirection(Direction new_direction);

    private:

        void Update(const mono::UpdateContext& context) override;
        void DrawElement(mono::IRenderer& renderer) const override;
        math::Quad LocalBoundingBox() const override;

        float m_fraction;
        float m_target_fraction;
        float m_velocity;
        Direction m_direction;
        math::Vector m_bar_size;

        std::unique_ptr<mono::IRenderBuffer> m_vertices;
        std::unique_ptr<mono::IRenderBuffer> m_background_colors;
        std::unique_ptr<mono::IRenderBuffer> m_foreground_colors;
        std::unique_ptr<mono::IElementBuffer> m_indices;
    };

    class UICheckboxElement : public UIElement
    {
    public: 

    };
}
