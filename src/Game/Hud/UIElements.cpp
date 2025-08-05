
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

#include "Debug/GameDebugVariables.h"

using namespace game;

UIElement::UIElement()
    : m_parent(nullptr)
    , m_scale(1.0f, 1.0f)
    , m_rotation(0.0f)
    , m_anchor_point(mono::AnchorPoint::CENTER)
    , m_show(true)
{ }

UIElement::~UIElement()
{
    for(UIElement* child : m_ui_elements)
        delete child;
}

void UIElement::Update(const mono::UpdateContext& context)
{
    for(UIElement* child : m_ui_elements)
        child->Update(context);
}

void UIElement::Draw(mono::IRenderer& renderer) const
{
    if(!m_show)
        return;

    const math::Matrix& transform = renderer.GetTransform() * LocalTransform();
    const auto transform_scope = mono::MakeTransformScope(transform, &renderer);

    DrawElement(renderer);

    for(const UIElement* ui : m_ui_elements)
        ui->Draw(renderer);

    if(g_draw_ui_element_bounds)
    {
        const math::Quad& bounds = BoundingBox();
        renderer.DrawQuad(bounds, mono::Color::CYAN, 1.0f);
    }
}

math::Quad UIElement::BoundingBox() const
{
    math::Quad bounds = LocalBoundingBox();
    //math::Quad bounds = math::Transformed(LocalTransform(), LocalBoundingBox());

    for(const UIElement* ui : m_ui_elements)
    {
        const math::Quad& local_bounds = ui->LocalBoundingBox();
        const math::Matrix& local_transform = ui->LocalTransform();
        math::ExpandBy(bounds, math::Transformed(local_transform, local_bounds));
        //math::ExpandBy(bounds, local_bounds);
    }

    //return math::Transformed(LocalTransform(), bounds);
    return bounds;
}

math::Quad UIElement::LocalBoundingBox() const
{
    //return math::Quad(m_position, 0.0f); //m_scale.x, m_scale.y);
    return math::ZeroQuad;
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

const math::Vector& UIElement::GetPosition() const
{
    return m_position;
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

void UIElement::SetAchorPoint(mono::AnchorPoint anchor_point)
{
    m_anchor_point = anchor_point;
}

math::Matrix UIElement::LocalTransform() const
{
    const math::Vector& anchor_offset = GetAnchorOffset();
    return 
        math::CreateMatrixWithPosition(m_position) *
        math::CreateMatrixWithPosition(anchor_offset) * 
        math::CreateMatrixFromZRotation(m_rotation) *
        math::CreateMatrixWithScale(m_scale);
}

math::Matrix UIElement::Transform() const
{
    return LocalTransform() * ((m_parent != nullptr) ? m_parent->Transform() : math::Matrix());
}

math::Vector UIElement::GetAnchorOffset() const
{
    const math::Quad& bounds = LocalBoundingBox();
    const math::Vector& anchor_offset = mono::CalculateOffsetFromCenter(m_anchor_point, math::Size(bounds), 1.0f);

    return anchor_offset;
}

void UIElement::AddChild(UIElement* element)
{
    m_ui_elements.push_back(element);
    element->m_parent = this;
}

void UIElement::RemoveChild(UIElement* element)
{
    mono::remove(m_ui_elements, element);
    element->m_parent = nullptr;
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

math::Quad UIOverlay::BoundingBox() const
{
    return math::InfQuad;
}

math::Quad UIOverlay::LocalBoundingBox() const
{
    return math::InfQuad;
}


UITextElement::UITextElement(int font_id, const std::string& text, const mono::Color::RGBA& color)
    : m_font_id(font_id)
    , m_text(text)
    , m_color(color)
    , m_shadow_color(mono::Color::BLACK)
    , m_shadow_offset(-0.1f, -0.1f)
    , m_draw_shadow(false)
{
    SetText(text);
}

void UITextElement::SetText(const std::string& new_text)
{
    m_text = new_text;
    if(!m_text.empty())
        m_draw_buffers = mono::BuildTextDrawBuffers(m_font_id, m_text.c_str(), mono::FontCentering::HORIZONTAL_VERTICAL);
}

void UITextElement::SetColor(const mono::Color::RGBA& new_color)
{
    m_color = new_color;
}

void UITextElement::SetShadowColor(const mono::Color::RGBA& shadow_color)
{
    m_shadow_color = shadow_color;
}

void UITextElement::SetShadowOffset(float x, float y)
{
    m_shadow_offset = math::Vector(x, y);
}

void UITextElement::SetEnableShadow(bool shadow)
{
    m_draw_shadow = shadow;
}

void UITextElement::SetAlpha(float alpha)
{
    m_color.alpha = alpha;
}

void UITextElement::DrawElement(mono::IRenderer& renderer) const
{
    if(m_text.empty())
        return;

    if(m_color.alpha == 0.0f)
        return;

    const mono::ITexturePtr texture = mono::GetFontTexture(m_font_id);

    if(m_draw_shadow)
    {
        math::Matrix shadow_world_transform = renderer.GetTransform();
        math::Translate(shadow_world_transform, m_shadow_offset);

        auto shadow_transform_scope = mono::MakeTransformScope(shadow_world_transform, &renderer);

        renderer.DrawGeometry(
            m_draw_buffers.vertices.get(),
            m_draw_buffers.uv.get(),
            m_draw_buffers.indices.get(),
            texture.get(),
            m_shadow_color,
            false,
            m_draw_buffers.indices->Size());
    }

    renderer.DrawGeometry(
        m_draw_buffers.vertices.get(),
        m_draw_buffers.uv.get(),
        m_draw_buffers.indices.get(),
        texture.get(),
        m_color,
        false,
        m_draw_buffers.indices->Size());
}

math::Quad UITextElement::LocalBoundingBox() const
{
    const mono::TextMeasurement text_measurement = mono::MeasureString(m_font_id, m_text.c_str());
    const math::Vector text_offset = mono::TextOffsetFromFontCentering(text_measurement.size, mono::FontCentering::HORIZONTAL_VERTICAL);
    return math::Quad(text_offset, text_offset + text_measurement.size);
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
        m_sprites.push_back(mono::RenderSystem::GetSpriteFactory()->CreateSprite(sprite_file.c_str()));
        m_sprite_buffers.push_back(mono::BuildSpriteDrawBuffers(m_sprites.back()->GetSpriteData()));

        const mono::ISprite* sprite = m_sprites.back().get();
        const mono::SpriteData* sprite_data = sprite->GetSpriteData();

        if(sprite_data)
        {
            math::Vector frame_size;

            for(const mono::SpriteFrame& frame : sprite_data->frames)
            {
                frame_size.x = std::max(frame_size.x, frame.size.x);
                frame_size.y = std::max(frame_size.y, frame.size.y);
            }

            m_sprite_bounds.push_back(math::Quad(math::ZeroVec, frame_size.x, frame_size.y));
        }
    }
}

void UISpriteElement::SetActiveSprite(uint32_t index, uint32_t animation_id)
{
    m_active_sprite = index;
    m_sprites[m_active_sprite]->SetAnimation(animation_id);
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

void UISpriteElement::DrawElement(mono::IRenderer& renderer) const
{
    if(m_sprites.empty() || m_sprite_buffers.empty())
        return;

    const mono::ISprite* sprite = m_sprites[m_active_sprite].get();
    const mono::SpriteDrawBuffers& buffers = m_sprite_buffers[m_active_sprite];
    renderer.DrawSprite(
        sprite, &buffers, m_indices.get(), sprite->GetCurrentFrameIndex() * buffers.vertices_per_sprite);
}

math::Quad UISpriteElement::LocalBoundingBox() const
{
    if(m_sprite_bounds.empty())
        return math::ZeroQuad;

    return m_sprite_bounds[m_active_sprite];
}


UISpriteBarElement::UISpriteBarElement()
    : m_spacing(0.1f)
{ }

void UISpriteBarElement::SetSpacing(float spacing)
{
    m_spacing = spacing;
}

void UISpriteBarElement::PushSprite(int sprite_handle, const char* sprite_file)
{
    const auto check_id = [sprite_handle](const UISpriteData& sprite_data) {
        return sprite_data.handle == sprite_handle;
    };
    const bool invalid_handle = mono::contains(m_sprites, check_id);
    if(invalid_handle)
        return;

    UISpriteElement* ui_sprite = new UISpriteElement(sprite_file);
    AddChild(ui_sprite);

    m_sprites.push_back(
        { sprite_handle, ui_sprite }
    );

    RecalculateLayout();
}

void UISpriteBarElement::RemoveSprite(int sprite_handle)
{
    const auto find_by_handle = [this, sprite_handle](const UISpriteData& sprite_data) {
        const bool this_is_the_one = (sprite_data.handle == sprite_handle);
        if(this_is_the_one)
            RemoveChild(sprite_data.ui_sprite);

        return this_is_the_one;
    };
    
    const bool element_removed = mono::remove_if(m_sprites, find_by_handle);

    if(element_removed)
        RecalculateLayout();
}

void UISpriteBarElement::RecalculateLayout()
{
    float offset_x = 0.0f;

    for(const UISpriteData& sprite_data : m_sprites)
    {
        sprite_data.ui_sprite->SetPosition(offset_x, 0.0f);

        const math::Quad& local_bounds = sprite_data.ui_sprite->LocalBoundingBox();
        offset_x += math::Width(local_bounds);
        offset_x += m_spacing;
    }
}


UITextureElement::UITextureElement()
{ }

UITextureElement::UITextureElement(const char* texture, float pixels_per_meter)
{
    SetTexture(texture, pixels_per_meter);
}

void UITextureElement::SetTexture(const char* texture, float pixels_per_meter)
{
    m_texture = mono::RenderSystem::GetTextureFactory()->CreateTexture(texture);

    mono::TextureBufferOptions options;
    options.pixels_per_meter = pixels_per_meter;
    options.anchor = mono::AnchorPoint::CENTER;
    m_draw_buffers = mono::BuildTextureDrawBuffers(m_texture.get(), options);

    m_pixels_per_meter = pixels_per_meter;
}

void UITextureElement::DrawElement(mono::IRenderer& renderer) const
{
    if(!m_texture)
        return;

    renderer.DrawGeometry(
        m_draw_buffers.vertices.get(),
        m_draw_buffers.uv.get(),
        m_draw_buffers.indices.get(),
        m_texture.get(),
        mono::Color::WHITE,
        false,
        m_draw_buffers.indices->Size());
}

math::Quad UITextureElement::LocalBoundingBox() const
{
    if(m_texture)
        return math::Quad(math::ZeroVec, m_texture->Width(), m_texture->Height()) / m_pixels_per_meter;

    return math::ZeroQuad;
}

math::Vector UITextureElement::GetTextureSize() const
{
    if(m_texture)
        return math::Vector(m_texture->Width(), m_texture->Height());

    return math::ZeroVec;
}


UITextureBoxElement::UITextureBoxElement()
    : m_pixels_per_meter(32.0f)
{
    m_left = new UITextureElement();
    m_mid = new UITextureElement();
    m_right = new UITextureElement();

    m_left->SetAchorPoint(mono::AnchorPoint::BOTTOM_RIGHT);
    m_right->SetAchorPoint(mono::AnchorPoint::BOTTOM_LEFT);

    AddChild(m_left);
    AddChild(m_mid);
    AddChild(m_right);
}

UITextureBoxElement::UITextureBoxElement(const char* left_sprite, const char* mid_sprite, const char* right_sprite, float pixels_per_meter)
    : UITextureBoxElement()
{
    SetTextures(left_sprite, mid_sprite, right_sprite, pixels_per_meter);
}

void UITextureBoxElement::SetTextures(const char* left_sprite, const char* mid_sprite, const char* right_sprite, float pixels_per_meter)
{
    m_left->SetTexture(left_sprite, pixels_per_meter);
    m_mid->SetTexture(mid_sprite, pixels_per_meter);
    m_right->SetTexture(right_sprite, pixels_per_meter);

    m_pixels_per_meter = pixels_per_meter;
}

void UITextureBoxElement::SetWidth(float width)
{
    m_mid->SetScale(math::Vector(width * m_pixels_per_meter, 1.0f));

    const math::Quad bounds = m_mid->BoundingBox();
    m_left->SetPosition(math::BottomLeft(bounds));
    m_right->SetPosition(math::BottomRight(bounds));
}


UISquareElement::UISquareElement(float width, float height, const mono::Color::RGBA& color)
    : UISquareElement(width, height, color, color, 0.0f)
{ }

UISquareElement::UISquareElement(
    float width, float height, const mono::Color::RGBA& color, const mono::Color::RGBA& border_color, float border_width)
    : m_width(width)
    , m_height(height)
    , m_border_width(border_width)
    , m_color(color)
    , m_border_color(border_color)
{
   const float half_width = width / 2.0f;
   const float half_height = height / 2.0f;

    const std::vector<math::Vector> vertex_data = {   
        { -half_width, -half_height }, 
        { -half_width, half_height },
        { half_width, half_height },
        { half_width, -half_height }
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

UISquareElement::~UISquareElement() = default;

void UISquareElement::DrawElement(mono::IRenderer& renderer) const
{
    renderer.DrawTrianges(m_vertices.get(), m_colors.get(), m_indices.get(), 0, 6);
    if(m_border_width > 0.0f)
        renderer.DrawPolyline(m_vertices.get(), m_border_colors.get(), m_indices.get(), 6, 5);
}

math::Quad UISquareElement::LocalBoundingBox() const
{
    return math::Quad(math::ZeroVec, m_width, m_height);
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



UICircleElement::UICircleElement(float radii, const mono::Color::RGBA& color)
    : m_radii(radii)
    , m_color(color)
{
   const float half_width = m_radii;
   const float half_height = m_radii;

    const std::vector<math::Vector> vertex_data = {
        { -half_width, -half_height }, 
        { -half_width, half_height },
        { half_width, half_height },
        { half_width, -half_height }
    };

    const std::vector<mono::Color::RGBA> color_data(4, color);

    m_vertices = mono::CreateRenderBuffer(mono::BufferType::STATIC, mono::BufferData::FLOAT, 2, 4, vertex_data.data());
    m_colors = mono::CreateRenderBuffer(mono::BufferType::STATIC, mono::BufferData::FLOAT, 4, 4, color_data.data());

    constexpr uint16_t indices[] = {
        0, 1, 2, 0, 2, 3,   // Two triangles
        0, 1, 2, 3, 0       // Outline
    };
    m_indices = mono::CreateElementBuffer(mono::BufferType::STATIC, std::size(indices), indices);
}

UICircleElement::~UICircleElement() = default;

void UICircleElement::DrawElement(mono::IRenderer& renderer) const
{
    renderer.DrawFilledCircle(math::ZeroVec, math::Vector(m_radii, m_radii), 16, m_color);
    //renderer.DrawTrianges(m_vertices.get(), m_colors.get(), m_indices.get(), 0, 6);
}

math::Quad UICircleElement::LocalBoundingBox() const
{
    return math::Quad(math::ZeroVec, m_radii, m_radii);
}


UIBarElement::UIBarElement(
    float background_width, float background_height, const mono::Color::RGBA& background_color, const mono::Color::RGBA& foreground_color)
    : m_fraction(0.0f)
    , m_target_fraction(0.0f)
    , m_velocity(0.0f)
    , m_direction(Direction::HORIZONTAL)
{
    m_bar_size = math::Vector(background_width, background_height);

    const std::vector<math::Vector> vertex_data = {
        { 0.0f, 0.0f },
        { 0.0f, background_height },
        { background_width, background_height },
        { background_width, 0.0f }
    };

/*
    const math::Vector half_size = m_bar_size / 2.0f;

    const std::vector<math::Vector> vertex_data = {
        { -half_size.x, -half_size.y },
        { -half_size.x, half_size.y },
        { half_size.x, half_size.y },
        { half_size.x, -half_size.y }
    };
*/

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

void UIBarElement::DrawElement(mono::IRenderer& renderer) const
{
    renderer.DrawTrianges(m_vertices.get(), m_background_colors.get(), m_indices.get(), 0, 6);

    const auto scale_transform_scope = mono::MakeTransformScope(
        renderer.GetTransform() * math::CreateMatrixWithScale(m_fraction, 1.0f), &renderer);
    renderer.DrawTrianges(m_vertices.get(), m_foreground_colors.get(), m_indices.get(), 0, 6);
}

math::Quad UIBarElement::LocalBoundingBox() const
{
    return math::Quad(math::ZeroVec, m_bar_size);
}
