
#include "HealthbarDrawer.h"
#include "FontIds.h"
#include "DamageSystem.h"

#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"
#include "Rendering/RenderBuffer/BufferFactory.h"
#include "Math/Quad.h"
#include "Util/Algorithm.h"
#include "TransformSystem/TransformSystem.h"
#include "EntitySystem/IEntityManager.h"

#include <limits>
#include <string>

using namespace game;

namespace
{
    struct Healthbar
    {
        math::Vector position;
        float health_percentage;
        float width;
        uint32_t last_damaged_timestamp;
        std::string name;
    };

    constexpr mono::Color::RGBA healthbar_red(1.0f, 0.3f, 0.3f, 1.0f);

    void GenerateHealthbarVertices(
        const std::vector<Healthbar>& healthbars,
        float bar_thickness,
        std::vector<math::Vector>& out_vertices,
        std::vector<mono::Color::RGBA>& out_colors,
        std::vector<uint16_t>& out_indices)
    {
        for(const Healthbar& bar : healthbars)
        {
            const uint16_t base_index = out_vertices.size();

            const float percentage_length = bar.width * bar.health_percentage;
            const float half_length = bar.width / 2.0f;
            const float half_thickness = bar_thickness / 2.0f;

            const math::Vector& top_left = bar.position - math::Vector(half_length, half_thickness);
            const math::Vector& bottom_left = bar.position - math::Vector(half_length, -half_thickness);

            const math::Vector& top_mid_right = top_left + math::Vector(percentage_length, 0.0f);
            const math::Vector& bottom_mid_right = bottom_left + math::Vector(percentage_length, 0.0f);

            const math::Vector& top_right = top_left + math::Vector(bar.width, 0.0f);
            const math::Vector& bottom_right = bottom_left + math::Vector(bar.width, 0.0f);

            out_vertices.emplace_back(bottom_left);
            out_vertices.emplace_back(top_left);
            out_vertices.emplace_back(top_mid_right);
            out_vertices.emplace_back(bottom_mid_right);
            out_vertices.emplace_back(top_mid_right);
            out_vertices.emplace_back(bottom_mid_right);
            out_vertices.emplace_back(top_right);
            out_vertices.emplace_back(bottom_right);

            out_colors.emplace_back(healthbar_red);
            out_colors.emplace_back(healthbar_red);
            out_colors.emplace_back(healthbar_red);
            out_colors.emplace_back(healthbar_red);
            out_colors.emplace_back(mono::Color::GRAY);
            out_colors.emplace_back(mono::Color::GRAY);
            out_colors.emplace_back(mono::Color::GRAY);
            out_colors.emplace_back(mono::Color::GRAY);

            out_indices.push_back(base_index + 0);
            out_indices.push_back(base_index + 1);
            out_indices.push_back(base_index + 2);

            out_indices.push_back(base_index + 0);
            out_indices.push_back(base_index + 2);
            out_indices.push_back(base_index + 3);

            out_indices.push_back(base_index + 5);
            out_indices.push_back(base_index + 4);
            out_indices.push_back(base_index + 6);

            out_indices.push_back(base_index + 5);
            out_indices.push_back(base_index + 6);
            out_indices.push_back(base_index + 7);
        }
    }
}

HealthbarDrawer::HealthbarDrawer(game::DamageSystem* damage_system, mono::TransformSystem* transform_system, mono::IEntityManager* entity_system)
    : m_damage_system(damage_system)
    , m_transform_system(transform_system)
    , m_entity_system(entity_system)
{ }

void HealthbarDrawer::Draw(mono::IRenderer& renderer) const
{
    constexpr uint32_t max_uint = std::numeric_limits<uint32_t>::max();
    const uint32_t timestamp = renderer.GetTimestamp();
    const math::Quad viewport = renderer.GetViewport();

    const float viewport_width = 10.0f;
    const float ratio = math::Width(viewport) / math::Height(viewport);
    const float viewport_height = viewport_width * ratio;

    std::vector<Healthbar> healthbars;
    std::vector<Healthbar> boss_healthbars;

    const auto collect_func = [&](uint32_t entity_id, DamageRecord& record) {
        const bool ignore_record = (record.last_damaged_timestamp == max_uint);
        if(ignore_record)
            return;

        const uint32_t delta = timestamp - record.last_damaged_timestamp;
        if(delta > 5000)
            return;

        if(record.health <= 0)
            return;

        Healthbar bar;
        bar.last_damaged_timestamp = record.last_damaged_timestamp;
        bar.health_percentage = float(record.health) / float(record.full_health);
        bar.name = m_entity_system->GetEntityName(entity_id);
        
        if(record.is_boss)
        {
            bar.position = math::ZeroVec;
            bar.width = viewport_width * 0.9f;
            boss_healthbars.push_back(bar);
        }
        else
        {
            const math::Quad& world_bb = m_transform_system->GetWorldBoundingBox(entity_id);
            bar.position = math::BottomCenter(world_bb);
            bar.width = std::max(math::Width(world_bb) * 1.25f, 0.5f);
            healthbars.push_back(bar);
        }
    };

    m_damage_system->ForEeach(collect_func);

    if(!boss_healthbars.empty())
    {
        const auto sort_on_timestamp = [](const Healthbar& first, const Healthbar& second) {
            return first.last_damaged_timestamp > second.last_damaged_timestamp;
        };
        std::sort(boss_healthbars.begin(), boss_healthbars.end(), sort_on_timestamp);
        boss_healthbars.erase(boss_healthbars.begin() +1, boss_healthbars.end());
    }

    const int n_healthbars = healthbars.size() + boss_healthbars.size();
    if(n_healthbars == 0)
        return;

    const uint32_t vertices_needed = n_healthbars * 8;
    const uint32_t indices_needed = n_healthbars * 12;

    std::vector<math::Vector> vertices;
    vertices.reserve(vertices_needed);

    std::vector<mono::Color::RGBA> colors;
    colors.reserve(vertices_needed);

    std::vector<uint16_t> indices;
    indices.reserve(indices_needed);

    GenerateHealthbarVertices(healthbars, 0.05f, vertices, colors, indices);
    const uint32_t n_healthbar_indices = indices.size();

    const float boss_healthbar_thickness = viewport_height * 0.035f;
    GenerateHealthbarVertices(boss_healthbars, boss_healthbar_thickness, vertices, colors, indices);
    const uint32_t n_boss_indices = indices.size() - n_healthbar_indices;

    auto vertex_buffer = mono::CreateRenderBuffer(
        mono::BufferType::STATIC, mono::BufferData::FLOAT, 2, vertices_needed, vertices.data());
    auto color_buffer = mono::CreateRenderBuffer(
        mono::BufferType::STATIC, mono::BufferData::FLOAT, 4, vertices_needed, colors.data());
    auto index_buffer = mono::CreateElementBuffer(
        mono::BufferType::STATIC, indices_needed, indices.data());

    if(!healthbars.empty())
        renderer.DrawTrianges(vertex_buffer.get(), color_buffer.get(), index_buffer.get(), 0, n_healthbar_indices);

    // Boss health bars

    if(!boss_healthbars.empty())
    {
        const math::Matrix projection = math::Ortho(0.0f, viewport_width, 0.0f, viewport_height, -10.0f, 10.0f);
        const mono::ScopedTransform projection_raii = mono::MakeProjectionScope(projection, &renderer);
        const mono::ScopedTransform view_transform = mono::MakeViewTransformScope(math::Matrix(), &renderer);

        math::Matrix transform;
        math::Translate(transform, math::Vector(viewport_width / 2.0f, viewport_height * 0.95f));
        const mono::ScopedTransform transform_scope = mono::MakeTransformScope(transform, &renderer);

        renderer.DrawTrianges(
            vertex_buffer.get(), color_buffer.get(), index_buffer.get(), n_healthbar_indices, n_boss_indices);

        const Healthbar& boss_healthbar = boss_healthbars.back();
        renderer.RenderText(
            FontId::RUSSOONE_TINY,
            boss_healthbar.name.c_str(),
            mono::Color::BLACK,
            mono::FontCentering::HORIZONTAL_VERTICAL);
    }
}

math::Quad HealthbarDrawer::BoundingBox() const
{
    return math::InfQuad;
}
