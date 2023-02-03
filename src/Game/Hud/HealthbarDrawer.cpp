
#include "HealthbarDrawer.h"
#include "FontIds.h"
#include "DamageSystem/DamageSystem.h"

#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"
#include "Rendering/RenderBuffer/BufferFactory.h"
#include "Math/Quad.h"
#include "Util/Algorithm.h"
#include "TransformSystem/TransformSystem.h"
#include "EntitySystem/IEntityManager.h"

#include "Rendering/RenderSystem.h"
#include "Rendering/Sprite/ISprite.h"

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
        float height;
        bool boss;
        uint32_t last_damaged_timestamp;
        std::string name;
    };

    constexpr mono::Color::RGBA healthbar_red = mono::Color::RGBA(1.0f, 0.3f, 0.3f, 1.0f);

    void GenerateHealthbarVertices(
        const std::vector<Healthbar>& healthbars,
        std::vector<math::Vector>& out_vertices,
        std::vector<mono::Color::RGBA>& out_colors,
        std::vector<uint16_t>& out_indices,
        std::vector<math::Vector>& out_boss_icon_positions)
    {
        for(const Healthbar& bar : healthbars)
        {
            const uint16_t base_index = out_vertices.size();

            const float percentage_length = bar.width * bar.health_percentage;
            const float half_length = bar.width / 2.0f;
            const float half_thickness = bar.height / 2.0f;

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

            if(bar.boss)
            {
                const math::Vector icon_position = bar.position - math::Vector(half_length, 0.0f);
                out_boss_icon_positions.push_back(icon_position);
            }
        }
    }
}

HealthbarDrawer::HealthbarDrawer(
    game::DamageSystem* damage_system, mono::TransformSystem* transform_system, mono::IEntityManager* entity_system)
    : m_damage_system(damage_system)
    , m_transform_system(transform_system)
    , m_entity_system(entity_system)
{
    m_boss_icon_sprite = mono::RenderSystem::GetSpriteFactory()->CreateSprite("res/sprites/squid.sprite");
    m_sprite_buffers = mono::BuildSpriteDrawBuffers(m_boss_icon_sprite->GetSpriteData());

    constexpr uint16_t indices[] = {
        0, 1, 2, 0, 2, 3
    };
    m_indices = mono::CreateElementBuffer(mono::BufferType::STATIC, 6, indices);
}

void HealthbarDrawer::Draw(mono::IRenderer& renderer) const
{
    constexpr uint32_t max_uint = std::numeric_limits<uint32_t>::max();
    const uint32_t timestamp = renderer.GetTimestamp();

    std::vector<Healthbar> healthbars;

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
        bar.boss = record.is_boss;
        
        const math::Quad& world_bb = m_transform_system->GetWorldBoundingBox(entity_id);
        bar.position = math::BottomCenter(world_bb);
        bar.width = std::max(math::Width(world_bb) * 1.25f, 0.5f);
        bar.height = bar.boss ? 0.075f : 0.05f;
        healthbars.push_back(bar);
    };

    m_damage_system->ForEeach(collect_func);

    const int n_healthbars = healthbars.size();
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

    std::vector<math::Vector> boss_icon_positions;

    GenerateHealthbarVertices(healthbars, vertices, colors, indices, boss_icon_positions);
    const uint32_t n_healthbar_indices = indices.size();

    auto vertex_buffer = mono::CreateRenderBuffer(
        mono::BufferType::STATIC, mono::BufferData::FLOAT, 2, vertices_needed, vertices.data());
    auto color_buffer = mono::CreateRenderBuffer(
        mono::BufferType::STATIC, mono::BufferData::FLOAT, 4, vertices_needed, colors.data());
    auto index_buffer = mono::CreateElementBuffer(
        mono::BufferType::STATIC, indices_needed, indices.data());

    if(!healthbars.empty())
        renderer.DrawTrianges(vertex_buffer.get(), color_buffer.get(), index_buffer.get(), 0, n_healthbar_indices);

    // Boss health bars

    for(const math::Vector& icon_position : boss_icon_positions)
    {
        const math::Matrix& world_transform = renderer.GetTransform() * math::CreateMatrixWithPosition(icon_position);
        auto transform_scope = mono::MakeTransformScope(world_transform, &renderer);
        renderer.DrawSprite(m_boss_icon_sprite.get(), &m_sprite_buffers, m_indices.get(), 0);
    }
}

math::Quad HealthbarDrawer::BoundingBox() const
{
    return math::InfQuad;
}
