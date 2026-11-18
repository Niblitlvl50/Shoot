
#include "GroundPaint.h"

#include "Math/MathFunctions.h"
#include "Rendering/GroundPaint.h"
#include "Rendering/IRenderer.h"
#include "Rendering/RenderSystem.h"
#include "Rendering/RenderBuffer/BufferFactory.h"
#include "Rendering/Texture/ITexture.h"
#include "System/File.h"
#include "System/System.h"

#include "stb/stb_image.h"
#include "stb/stb_image_write.h"

#include <algorithm>
#include <cmath>

namespace
{
    // Keeps a mistyped size from allocating gigabytes.
    constexpr int max_splat_size = 4096;

    int TexelCount(float meters, float texels_per_meter)
    {
        const int count = int(std::ceil(meters * texels_per_meter));
        return std::clamp(count, 1, max_splat_size);
    }
}

static_assert(game::GROUND_PAINT_LAYERS == mono::GROUND_PAINT_LAYERS);

using namespace game;

std::string game::GroundPaintFilename(const std::string& world_filename)
{
    const std::string extension = ".components";
    const bool has_extension =
        world_filename.size() > extension.size() &&
        world_filename.compare(world_filename.size() - extension.size(), extension.size(), extension) == 0;

    const std::string base_name = has_extension ? world_filename.substr(0, world_filename.size() - extension.size()) : world_filename;
    return base_name + ".splat.png";
}

bool game::IsGroundPaintEnabled(const GroundPaintSettings& settings)
{
    return settings.size.x > 0.0f && settings.size.y > 0.0f && settings.texels_per_meter > 0.0f;
}

void GroundPaintData::Load(const std::string& filename, const GroundPaintSettings& settings)
{
    m_position = settings.position;
    m_size = settings.size;
    m_texels_per_meter = settings.texels_per_meter;
    m_width = TexelCount(settings.size.x, settings.texels_per_meter);
    m_height = TexelCount(settings.size.y, settings.texels_per_meter);
    m_pixels.assign(m_width * m_height * 4, 0);
    m_dirty = true;

    if(!file::Exists(filename.c_str()))
        return;

    int file_width = 0;
    int file_height = 0;
    int file_components = 0;
    stbi_uc* file_pixels = stbi_load(filename.c_str(), &file_width, &file_height, &file_components, 4);
    if(!file_pixels)
    {
        System::Log("GroundPaint|Unable to load '%s'.", filename.c_str());
        return;
    }

    // The png has its top row first.
    for(int y = 0; y < m_height; ++y)
    {
        const int file_y = (file_height - 1) - (y * file_height / m_height);
        for(int x = 0; x < m_width; ++x)
        {
            const int file_x = x * file_width / m_width;
            const stbi_uc* source = file_pixels + (file_y * file_width + file_x) * 4;
            std::copy(source, source + 4, &m_pixels[(y * m_width + x) * 4]);
        }
    }

    stbi_image_free(file_pixels);
}

bool GroundPaintData::Save(const std::string& filename) const
{
    if(m_pixels.empty())
        return false;

    // The rows go from the bottom up, the png wants the top row first.
    stbi_flip_vertically_on_write(1);
    const int result = stbi_write_png(filename.c_str(), m_width, m_height, 4, m_pixels.data(), m_width * 4);
    stbi_flip_vertically_on_write(0);

    if(result == 0)
        System::Log("GroundPaint|Unable to save '%s'.", filename.c_str());

    return (result != 0);
}

void GroundPaintData::Resize(const GroundPaintSettings& settings)
{
    const math::Vector old_position = m_position;
    const float old_texels_per_meter = m_texels_per_meter;
    const int old_width = m_width;
    const int old_height = m_height;
    const std::vector<uint8_t> old_pixels = std::move(m_pixels);

    m_position = settings.position;
    m_size = settings.size;
    m_texels_per_meter = settings.texels_per_meter;
    m_width = TexelCount(settings.size.x, settings.texels_per_meter);
    m_height = TexelCount(settings.size.y, settings.texels_per_meter);
    m_pixels.assign(m_width * m_height * 4, 0);
    m_dirty = true;

    if(old_pixels.empty() || old_texels_per_meter <= 0.0f)
        return;

    for(int y = 0; y < m_height; ++y)
    {
        for(int x = 0; x < m_width; ++x)
        {
            const math::Vector world_position = m_position + math::Vector(x + 0.5f, y + 0.5f) / m_texels_per_meter;
            const math::Vector old_texel = (world_position - old_position) * old_texels_per_meter;
            const int old_x = int(std::floor(old_texel.x));
            const int old_y = int(std::floor(old_texel.y));
            if(old_x < 0 || old_x >= old_width || old_y < 0 || old_y >= old_height)
                continue;

            const uint8_t* source = &old_pixels[(old_y * old_width + old_x) * 4];
            std::copy(source, source + 4, &m_pixels[(y * m_width + x) * 4]);
        }
    }
}

bool GroundPaintData::MatchesSettings(const GroundPaintSettings& settings) const
{
    return m_position == settings.position && m_size == settings.size && m_texels_per_meter == settings.texels_per_meter;
}

void GroundPaintData::Paint(const math::Vector& world_position, float radius, float strength, int layer)
{
    if(m_pixels.empty() || radius <= 0.0f)
        return;

    const math::Vector center_texel = (world_position - m_position) * m_texels_per_meter;
    const float radius_texels = radius * m_texels_per_meter;

    const int min_x = std::max(0, int(std::floor(center_texel.x - radius_texels)));
    const int max_x = std::min(m_width - 1, int(std::ceil(center_texel.x + radius_texels)));
    const int min_y = std::max(0, int(std::floor(center_texel.y - radius_texels)));
    const int max_y = std::min(m_height - 1, int(std::ceil(center_texel.y + radius_texels)));

    const int painted_channel = layer - 1;

    for(int y = min_y; y <= max_y; ++y)
    {
        for(int x = min_x; x <= max_x; ++x)
        {
            const math::Vector texel_center = math::Vector(x + 0.5f, y + 0.5f);
            const float distance = math::Length(texel_center - center_texel) / radius_texels;
            if(distance >= 1.0f)
                continue;

            const float falloff = 1.0f - (distance * distance);
            const float amount = std::clamp(strength * falloff, 0.0f, 1.0f);

            // Raising one layer lowers the others, so the weights never add up to more than one.
            uint8_t* texel = &m_pixels[(y * m_width + x) * 4];
            for(int channel = 0; channel < 4; ++channel)
            {
                const float value = float(texel[channel]);
                if(channel == painted_channel)
                    texel[channel] = uint8_t(std::min(255.0f, std::ceil(value + (255.0f - value) * amount)));
                else
                    texel[channel] = uint8_t(std::floor(value * (1.0f - amount)));
            }
        }
    }

    m_dirty = true;
}

int GroundPaintData::Width() const
{
    return m_width;
}

int GroundPaintData::Height() const
{
    return m_height;
}

const std::vector<uint8_t>& GroundPaintData::Pixels() const
{
    return m_pixels;
}


GroundPaintDrawer::GroundPaintDrawer(const GroundPaintSettings& settings, std::shared_ptr<GroundPaintData> data)
    : m_settings(settings)
    , m_data(std::move(data))
{ }

void GroundPaintDrawer::Draw(mono::IRenderer& renderer) const
{
    if(!IsGroundPaintEnabled(m_settings))
        return;

    if(!m_data->MatchesSettings(m_settings))
        m_data->Resize(m_settings);

    const mono::ITextureFactory* texture_factory = mono::RenderSystem::GetTextureFactory();

    for(int index = 0; index < GROUND_PAINT_LAYERS; ++index)
    {
        if(m_texture_names[index] == m_settings.textures[index])
            continue;

        m_texture_names[index] = m_settings.textures[index];
        m_textures[index] = m_texture_names[index].empty() ? nullptr : texture_factory->CreateTexture(m_texture_names[index].c_str());
    }

    const bool has_any_texture = std::any_of(std::begin(m_textures), std::end(m_textures), [](const mono::ITexturePtr& texture) {
        return texture != nullptr;
    });
    if(!has_any_texture)
        return;

    const bool splat_size_changed =
        !m_splat_texture ||
        int(m_splat_texture->Width()) != m_data->Width() ||
        int(m_splat_texture->Height()) != m_data->Height();
    if(splat_size_changed)
    {
        m_splat_texture = texture_factory->CreateDynamicTexture(m_data->Width(), m_data->Height(), 4);
        m_data->m_dirty = true;
    }

    if(m_data->m_dirty)
    {
        m_splat_texture->UpdateData(m_data->Pixels().data());
        m_data->m_dirty = false;
    }

    const math::Quad quad = math::Quad(m_settings.position, m_settings.position + m_settings.size);
    const bool quad_changed = (quad.bottom_left != m_quad.bottom_left || quad.top_right != m_quad.top_right);
    if(!m_vertices || quad_changed)
    {
        m_quad = quad;

        const math::Vector vertices[] = {
            quad.bottom_left,
            math::Vector(quad.bottom_left.x, quad.top_right.y),
            quad.top_right,
            math::Vector(quad.top_right.x, quad.bottom_left.y),
        };
        constexpr uint16_t indices[] = { 0, 1, 2, 0, 2, 3 };

        m_vertices = mono::CreateRenderBuffer(
            mono::BufferType::STATIC, mono::BufferData::FLOAT, 2, std::size(vertices), vertices, "ground_paint_drawer");
        m_indices = mono::CreateElementBuffer(mono::BufferType::STATIC, std::size(indices), indices, "ground_paint_drawer");
    }

    // A layer without a texture shows the base layer, or nothing if the base has none either.
    mono::GroundPaintDrawParams params;
    params.splat_map = m_splat_texture.get();
    params.splat_rect = quad;
    params.edge_roughness = m_settings.edge_roughness;
    params.blend_layers = m_settings.blend_layers;
    for(int index = 0; index < GROUND_PAINT_LAYERS; ++index)
    {
        const bool has_texture = (m_textures[index] != nullptr);
        params.layer_textures[index] = has_texture ? m_textures[index].get() : m_textures[0].get();
        params.layer_scales[index] = has_texture ? m_settings.texture_scales[index] : m_settings.texture_scales[0];
    }

    renderer.DrawGroundPaint(m_vertices.get(), m_indices.get(), params);
}

math::Quad GroundPaintDrawer::BoundingBox() const
{
    return math::Quad(m_settings.position, m_settings.position + m_settings.size);
}
