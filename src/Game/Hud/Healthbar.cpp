
#include "Healthbar.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"
#include "Math/Quad.h"

#include "DamageSystem.h"
#include "TransformSystem/TransformSystem.h"

#include "System/System.h"

#include <limits>

using namespace game;

namespace
{
    struct Healthbar
    {
        math::Vector position;
        float health_percentage;
        float width;
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

HealthbarDrawer::HealthbarDrawer(game::DamageSystem* damage_system, mono::TransformSystem* transform_system)
    : m_damage_system(damage_system)
    , m_transform_system(transform_system)
{ }

void HealthbarDrawer::Draw(mono::IRenderer& renderer) const
{
    constexpr uint32_t max_uint = std::numeric_limits<uint32_t>::max();
    const uint32_t now = System::GetMilliseconds();

    std::vector<Healthbar> healthbars;

    const auto collect_func = [this, now, &healthbars](uint32_t entity_id, DamageRecord& record) {
        const bool ignore_record = record.last_damaged_timestamp == max_uint;
        if(ignore_record)
            return;

        const uint32_t delta = now - record.last_damaged_timestamp;
        if(delta > 5000)
            return;

        const math::Matrix& transform = m_transform_system->GetTransform(entity_id);
        const math::Vector& position = math::GetPosition(transform);
        const math::Vector& scale = math::Vector(1.0f, 1.0f);

        Healthbar bar;
        bar.position = position - math::Vector(0.0f, scale.y / 2.0f + 0.5f);
        bar.width = scale.x;
        bar.health_percentage = float(record.health) / float(record.full_health);
        healthbars.push_back(bar);
    };

    m_damage_system->ForEeach(collect_func);

    constexpr float line_width = 4.0f;
    constexpr mono::Color::RGBA background_color(0.5f, 0.5f, 0.5f, 1.5f);
    constexpr mono::Color::RGBA healthbar_color(1.0f, 0.3f, 0.3f, 1.0f);

    const std::vector<math::Vector>& background_lines = GenerateHealthbarVertices(healthbars, true);
    const std::vector<math::Vector>& healthbar_lines = GenerateHealthbarVertices(healthbars, false);

    renderer.DrawLines(background_lines, background_color, line_width);
    renderer.DrawLines(healthbar_lines, healthbar_color, line_width);
}

math::Quad HealthbarDrawer::BoundingBox() const
{
    return math::InfQuad;
}
