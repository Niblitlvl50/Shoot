
#include "UIElements.h"

#include "Rendering/RenderSystem.h"
#include "Rendering/IRenderer.h"
#include "Rendering/RenderBuffer/BufferFactory.h"
#include "Rendering/Sprite/ISprite.h"
#include "Rendering/Sprite/ISpriteFactory.h"
#include "Rendering/Sprite/SpriteBufferFactory.h"
#include "Rendering/Texture/ITexture.h"

#include "Rendering/Text/TextBufferFactory.h"
#include "Rendering/Text/TextFunctions.h"

#include "Math/CriticalDampedSpring.h"
#include "Util/Algorithm.h"

using namespace game;

UIElement::UIElement()
    : m_scale(1.0f, 1.0f)
    , m_rotation(0.0f)
    , m_show(true)
{ }

void UIElement::Update(const mono::UpdateContext& context)
{
    for(UIElement* child : m_ui_elements)
        child->Update(context);
}

void UIElement::Draw(mono::IRenderer& renderer) const
{
    if(!m_show)
        return;

    const math::Matrix& transform = renderer.GetTransform() * Transform();
    const auto transform_scope = mono::MakeTransformScope(transform, &renderer);

    for(const UIElement* ui : m_ui_elements)
        ui->Draw(renderer);
}

math::Quad UIElement::BoundingBox() const
{
    return math::InfQuad;
}

void UIElement::Show()
{
    m_show = true;
}
void UIElement::Hide()
{
    m_show = false;
}

void UIElement::SetPosition(const math::Vector& position)
{
    m_position = position;
}

void UIElement::SetPosition(float x, float y)
{
    m_position.x = x;
    m_position.y = y;
}

void UIElement::SetScale(const math::Vector& scale)
{
    m_scale = scale;
}

void UIElement::SetScale(float uniform_scale)
{
    m_scale.x = m_scale.y = uniform_scale;
}

void UIElement::SetRotation(float radians)
{
    m_rotation = radians;
}

math::Matrix UIElement::Transform() const
{
    return 
        math::CreateMatrixWithPosition(m_position) *
        math::CreateMatrixFromZRotation(m_rotation) *
        math::CreateMatrixWithScale(m_scale);
}

void UIElement::AddChild(UIElement* element)
{
    m_ui_elements.push_back(element);
}

void UIElement::RemoveChild(UIElement* element)
{
    mono::remove(m_ui_elements, element);
}


UIOverlay::UIOverlay(float width, float height)
    : m_width(width)
    , m_height(height)
    , m_projection(math::Ortho(0, width, 0, height, -10, 10))
{ }

void UIOverlay::Draw(mono::IRenderer& renderer) const
{
    const auto projection_scope = mono::MakeProjectionScope(m_projection, &renderer);
    const auto view_scope = mono::MakeViewTransformScope(math::Matrix(), &renderer);
    UIElement::Draw(renderer);
}


UITextElement::UITextElement(int font_id, const std::string& text, mono::FontCentering centering, const mono::Color::RGBA& color)
    : m_font_id(font_id)
    , m_text(text)
    , m_centering(centering)
    , m_color(color)
{
    SetText(text);
}

void UITextElement::SetText(const std::string& new_text)
{
    m_text = new_text;
    if(!m_text.empty())
        m_draw_buffers = mono::BuildTextDrawBuffers(m_font_id, m_text.c_str(), m_centering);
}

void UITextElement::SetColor(const mono::Color::RGBA& new_color)
{
    m_color = new_color;
}

void UITextElement::SetAlpha(float alpha)
{
    m_color.alpha = alpha;
}

void UITextElement::Draw(mono::IRenderer& renderer) const
{
    if(!m_show || m_text.empty())
        return;

    UIElement::Draw(renderer);

    if(m_color.alpha == 0.0f)
        return;

    const mono::ITexturePtr texture = mono::GetFontTexture(m_font_id);

    const math::Matrix& transform = renderer.GetTransform() * Transform();
    const auto transform_scope = mono::MakeTransformScope(transform, &renderer);

    renderer.RenderText(
        m_draw_buffers.vertices.get(), m_draw_buffers.uv.get(), m_draw_buffers.indices.get(), texture.get(), m_color);
}

UISpriteElement::UISpriteElement()
    : m_active_sprite(0)
{
    constexpr uint16_t indices[] = {
        0, 1, 2, 0, 2, 3
    };
    m_indices = mono::CreateElementBuffer(mono::BufferType::STATIC, 6, indices);
}

UISpriteElement::UISpriteElement(const std::string& sprite_file)
    : UISpriteElement()
{
    SetSprite(sprite_file);
}

UISpriteElement::UISpriteElement(const std::vector<std::string>& sprite_files)
    : UISpriteElement()
{
    SetSprites(sprite_files);
}

void UISpriteElement::SetSprite(const std::string& sprite_file)
{
    SetSprites({ sprite_file });
}

void UISpriteElement::SetSprites(const std::vector<std::string>& sprite_files)
{
    m_sprites.clear();
    m_sprite_buffers.clear();

    for(const std::string& sprite_file : sprite_files)
    {
        m_sprites.push_back(mono::GetSpriteFactory()->CreateSprite(sprite_file.c_str()));
        m_sprite_buffers.push_back(mono::BuildSpriteDrawBuffers(m_sprites.back()->GetSpriteData()));
    }
}

void UISpriteElement::SetActiveSprite(uint32_t index)
{
    m_active_sprite = index;
}

mono::ISprite* UISpriteElement::GetSprite(uint32_t index)
{
    return m_sprites[index].get();
}

void UISpriteElement::Update(const mono::UpdateContext& update_context)
{
    UIElement::Update(update_context);

    for(auto& sprite : m_sprites)
        sprite->Update(update_context);
}

void UISpriteElement::Draw(mono::IRenderer& renderer) const
{
    if(!m_show)
        return;

    UIElement::Draw(renderer);

    if(m_sprites.empty() || m_sprite_buffers.empty())
        return;

    const math::Matrix& transform = renderer.GetTransform() * Transform();
    const auto transform_scope = mono::MakeTransformScope(transform, &renderer);

    const mono::ISprite* sprite = m_sprites[m_active_sprite].get();
    const mono::SpriteDrawBuffers& buffers = m_sprite_buffers[m_active_sprite];
    renderer.DrawSprite(
        sprite,
        buffers.vertices.get(),
        buffers.offsets.get(),
        buffers.uv.get(),
        buffers.uv_flipped.get(),
        buffers.heights.get(),
        m_indices.get(),
        sprite->GetTexture(),
        sprite->GetCurrentFrameIndex() * buffers.vertices_per_sprite);
}


UITextureElement::UITextureElement(const char* texture)
{
    m_texture = mono::GetTextureFactory()->CreateTexture(texture);
    m_draw_buffers = mono::BuildTextureDrawBuffers(m_texture.get());
}

void UITextureElement::Draw(mono::IRenderer& renderer) const
{
    if(!m_show)
        return;

    UIElement::Draw(renderer);

    const math::Matrix& transform = renderer.GetTransform() * Transform();
    const auto transform_scope = mono::MakeTransformScope(transform, &renderer);
    renderer.DrawGeometry(
        m_draw_buffers.vertices.get(),
        m_draw_buffers.uv.get(),
        m_draw_buffers.indices.get(),
        m_texture.get(),
        mono::Color::WHITE,
        false,
        m_draw_buffers.indices->Size());
}


UISquareElement::UISquareElement(float width, float height, const mono::Color::RGBA& color)
    : UISquareElement(width, height, color, color, 0.0f)
{ }

UISquareElement::UISquareElement(
    float width, float height, const mono::Color::RGBA& color, const mono::Color::RGBA& border_color, float border_width)
    : m_border_width(border_width)
    , m_color(color)
    , m_border_color(border_color)
{
    const std::vector<math::Vector> vertex_data = {
        { 0.0f, 0.0f },
        { 0.0f, height },
        { width, height },
        { width, 0.0f }
    };

    m_vertices = mono::CreateRenderBuffer(mono::BufferType::STATIC, mono::BufferData::FLOAT, 2, 4, vertex_data.data());

    SetColor(m_color);
    SetBorderColor(m_border_color);

    constexpr uint16_t indices[] = {
        0, 1, 2, 0, 2, 3,   // Two triangles
        0, 1, 2, 3, 0       // Outline
    };
    m_indices = mono::CreateElementBuffer(mono::BufferType::STATIC, std::size(indices), indices);
}

void UISquareElement::Draw(mono::IRenderer& renderer) const
{
    if(!m_show)
        return;

    UIElement::Draw(renderer);

    const math::Matrix& transform = renderer.GetTransform() * Transform();
    const auto transform_scope = mono::MakeTransformScope(transform, &renderer);

    renderer.DrawTrianges(m_vertices.get(), m_colors.get(), m_indices.get(), 0, 6);
    if(m_border_width > 0.0f)
        renderer.DrawPolyline(m_vertices.get(), m_border_colors.get(), m_indices.get(), 6, 5);
}

void UISquareElement::SetColor(const mono::Color::RGBA& color)
{
    const std::vector<mono::Color::RGBA> color_data(4, color);
    m_colors = mono::CreateRenderBuffer(mono::BufferType::STATIC, mono::BufferData::FLOAT, 4, 4, color_data.data());
}

const mono::Color::RGBA& UISquareElement::GetColor() const
{
    return m_color;
}

void UISquareElement::SetBorderColor(const mono::Color::RGBA& color)
{
    const std::vector<mono::Color::RGBA> border_color_data(4, color);
    m_border_colors = mono::CreateRenderBuffer(mono::BufferType::STATIC, mono::BufferData::FLOAT, 4, 4, border_color_data.data());
}

const mono::Color::RGBA& UISquareElement::GetBorderColor() const
{
    return m_border_color;
}



UIBarElement::UIBarElement(
    float background_width, float background_height, const mono::Color::RGBA& background_color,
    float foreground_width, float foreground_height, const mono::Color::RGBA& foreground_color)
    : m_fraction(0.0f)
    , m_target_fraction(0.0f)
    , m_velocity(0.0f)
    , m_direction(Direction::HORIZONTAL)
{
    const std::vector<math::Vector> vertex_data = {
        { 0.0f, 0.0f },
        { 0.0f, background_height },
        { background_width, background_height },
        { background_width, 0.0f }
    };

    const std::vector<mono::Color::RGBA> background_color_data(4, background_color);
    const std::vector<mono::Color::RGBA> foreground_color_data(4, foreground_color);

    constexpr uint16_t indices[] = {
        0, 1, 2, 0, 2, 3,   // Two triangles
    };

    m_vertices = mono::CreateRenderBuffer(mono::BufferType::STATIC, mono::BufferData::FLOAT, 2, 4, vertex_data.data());
    m_background_colors = mono::CreateRenderBuffer(mono::BufferType::STATIC, mono::BufferData::FLOAT, 4, 4, background_color_data.data());
    m_foreground_colors = mono::CreateRenderBuffer(mono::BufferType::STATIC, mono::BufferData::FLOAT, 4, 4, foreground_color_data.data());
    m_indices = mono::CreateElementBuffer(mono::BufferType::STATIC, std::size(indices), indices);
}

void UIBarElement::SetFraction(float fraction)
{
    m_target_fraction = fraction;
}

void UIBarElement::SetDirection(Direction new_direction)
{
    m_direction = new_direction;
}

void UIBarElement::Update(const mono::UpdateContext& context)
{
    UIElement::Update(context);
    math::simple_spring_damper_implicit(m_fraction, m_velocity, m_target_fraction, 0.1f, context.delta_s);
}

void UIBarElement::Draw(mono::IRenderer& renderer) const
{
    if(!m_show)
        return;

    UIElement::Draw(renderer);

    const math::Matrix& transform = renderer.GetTransform() * Transform();
    const auto transform_scope = mono::MakeTransformScope(transform, &renderer);

    renderer.DrawTrianges(m_vertices.get(), m_background_colors.get(), m_indices.get(), 0, 6);

    const auto scale_transform_scope = mono::MakeTransformScope(
        transform * math::CreateMatrixWithScale(m_fraction, 1.0f), &renderer);
    renderer.DrawTrianges(m_vertices.get(), m_foreground_colors.get(), m_indices.get(), 0, 6);
}
