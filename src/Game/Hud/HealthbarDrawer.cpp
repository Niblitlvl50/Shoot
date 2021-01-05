
#include "HealthbarDrawer.h"
#include "FontIds.h"
#include "DamageSystem.h"

#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"
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

    std::vector<math::Vector> GenerateHealthbarVertices(const std::vector<Healthbar>& healthbars, bool full_width)
    {
        std::vector<math::Vector> vertices;
        vertices.reserve(healthbars.size() * 2);

        for(const Healthbar& bar : healthbars)
        {
            const math::Vector shift_vector(bar.width / 2.0f, 0.0f);

            const math::Vector& left = bar.position - shift_vector;
            const math::Vector& right = bar.position + shift_vector;
            const math::Vector& diff = right - left;

            const float percentage = full_width ? 1.0f : bar.health_percentage;

            vertices.emplace_back(left);
            vertices.emplace_back(left + (diff * percentage));
        }

        return vertices;
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

    std::vector<Healthbar> healthbars;
    std::vector<Healthbar> boss_healthbars;

    const auto collect_func = [&](uint32_t entity_id, DamageRecord& record) {
        const bool ignore_record = (record.last_damaged_timestamp == max_uint);
        if(ignore_record)
            return;

        const uint32_t delta = timestamp - record.last_damaged_timestamp;
        if(delta > 5000)
            return;

        Healthbar bar;
        bar.last_damaged_timestamp = record.last_damaged_timestamp;
        bar.health_percentage = float(record.health) / float(record.full_health);
        bar.name = m_entity_system->GetEntityName(entity_id);
        
        if(record.is_boss)
        {
            bar.position = math::TopCenter(viewport) + math::Vector(0.0f, -1.0f);
            bar.width = math::Width(viewport) * 0.9f;
            boss_healthbars.push_back(bar);
        }
        else
        {
            const math::Quad& world_bb = m_transform_system->GetWorldBoundingBox(entity_id);

            bar.position = math::BottomCenter(world_bb);
            bar.width = 1.0f;
            healthbars.push_back(bar);
        }
    };

    m_damage_system->ForEeach(collect_func);

    constexpr float line_width = 4.0f;
    constexpr mono::Color::RGBA background_color(0.5f, 0.5f, 0.5f, 1.0f);
    constexpr mono::Color::RGBA healthbar_color(1.0f, 0.3f, 0.3f, 1.0f);

    const std::vector<math::Vector>& background_lines = GenerateHealthbarVertices(healthbars, true);
    const std::vector<math::Vector>& healthbar_lines = GenerateHealthbarVertices(healthbars, false);

    renderer.DrawLines(background_lines, background_color, line_width);
    renderer.DrawLines(healthbar_lines, healthbar_color, line_width);


    // Boss health bars

    if(!boss_healthbars.empty())
    {
        const auto sort_on_timestamp = [](const Healthbar& first, const Healthbar& second) {
            return first.last_damaged_timestamp > second.last_damaged_timestamp;
        };
        std::sort(boss_healthbars.begin(), boss_healthbars.end(), sort_on_timestamp);
        boss_healthbars.erase(boss_healthbars.begin() +1, boss_healthbars.end());

        const std::vector<math::Vector>& boss_background_lines = GenerateHealthbarVertices(boss_healthbars, true);
        const std::vector<math::Vector>& boss_healthbar_lines = GenerateHealthbarVertices(boss_healthbars, false);

        constexpr float line_width_boss = 8.0f;

        const math::Matrix projection = math::Ortho(viewport.mA.x, viewport.mB.x, viewport.mA.y, viewport.mB.y, -10.0f, 10.0f);

        const mono::ScopedTransform projection_raii = mono::MakeProjectionScope(projection, &renderer);
        const mono::ScopedTransform view_transform = mono::MakeViewTransformScope(math::Matrix(), &renderer);
        const mono::ScopedTransform transform_scope = mono::MakeTransformScope(math::Matrix(), &renderer);

        renderer.DrawLines(boss_background_lines, background_color, line_width_boss);
        renderer.DrawLines(boss_healthbar_lines, healthbar_color, line_width_boss);

        const Healthbar& boss_healthbar = boss_healthbars.back();
        renderer.DrawText(
            shared::FontId::PIXELETTE_TINY, boss_healthbar.name.c_str(), boss_healthbar.position + math::Vector(0.0f, 0.1f), true, mono::Color::BLACK);
    }
}

math::Quad HealthbarDrawer::BoundingBox() const
{
    return math::InfQuad;
}
