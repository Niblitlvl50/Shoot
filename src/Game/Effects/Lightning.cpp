
#include "Lightning.h"
#include "Util/Random.h"

#include <algorithm>

std::vector<math::Vector> game::GenerateLightning(const math::Vector& from, const math::Vector& to, uint32_t points_per_meter)
{
    const math::Vector diff = to - from;
    const math::Vector perpendicular = math::Normalized(math::Vector(diff.y, -diff.x));

    const float diff_length = math::Length(diff);

    const uint32_t n_points = diff_length * points_per_meter;

    std::vector<float> t_points(n_points, 0.0f);
    for(uint32_t index = 1; index < t_points.size(); ++index)
        t_points[index] = mono::Random();

    t_points.back() = 1.0f;

    std::sort(t_points.begin(), t_points.end());

    std::vector<math::Vector> points;
    points.push_back(from);

    const float sway = 1.0f;
    const float jaggedness = 1.0f / sway;

    math::Vector prevPoint = from;
    float prevDisplacement = 0.0f;

    for(uint32_t i = 1; i < t_points.size(); ++i)
    {
        const float pos = t_points[i];

        // used to prevent sharp angles by ensuring very close positions also have small perpendicular variation.
        const float scale = (diff_length * jaggedness) * (pos - t_points[i - 1]);

        // defines an envelope. Points near the middle of the bolt can be further from the central line.
        const float envelope = pos > 0.95f ? 2.0f * (1.0f - pos) : 1.0f;

        float displacement = mono::Random(-sway, sway);
        displacement -= (displacement - prevDisplacement) * (1.0f - scale);
        displacement *= envelope;

        const math::Vector point = from + (diff * pos) + (perpendicular * displacement);
        points.push_back(point);

        prevPoint = point;
        prevDisplacement = displacement;
    }

    return points;
}