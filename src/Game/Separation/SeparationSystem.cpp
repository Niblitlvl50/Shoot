
#include "SeparationSystem.h"

#include "Physics/IBody.h"
#include "Math/MathFunctions.h"
#include "IUpdatable.h"

#include <algorithm>
#include <cmath>

namespace tweak_values
{
    constexpr float radius = 1.0f;
    constexpr float strength = 0.5f;
}

using namespace game;

void SeparationSystem::Register(uint32_t entity_id, mono::IBody* body)
{
    m_agents.push_back({ entity_id, body, math::ZeroVec });
}

void SeparationSystem::Unregister(uint32_t entity_id)
{
    const auto it = std::find_if(m_agents.begin(), m_agents.end(), [entity_id](const Agent& a) {
        return a.entity_id == entity_id;
    });
    if(it != m_agents.end())
        m_agents.erase(it);
}

math::Vector SeparationSystem::GetSeparation(uint32_t entity_id) const
{
    const auto it = std::find_if(m_agents.begin(), m_agents.end(), [entity_id](const Agent& a) {
        return a.entity_id == entity_id;
    });
    return (it != m_agents.end()) ? it->separation : math::ZeroVec;
}

const char* SeparationSystem::Name() const
{
    return "separationsystem";
}

uint64_t SeparationSystem::CellKey(int32_t cx, int32_t cy)
{
    return uint64_t(uint32_t(cx)) | (uint64_t(uint32_t(cy)) << 32);
}

void SeparationSystem::Update(const mono::UpdateContext& update_context)
{
    if(m_agents.size() < 2)
        return;

    m_grid.clear();

    const float inv_cell = 1.0f / tweak_values::radius;
    for(uint32_t i = 0; i < uint32_t(m_agents.size()); ++i)
    {
        const math::Vector pos = m_agents[i].body->GetPosition();
        const int32_t cx = int32_t(std::floor(pos.x * inv_cell));
        const int32_t cy = int32_t(std::floor(pos.y * inv_cell));
        m_grid[CellKey(cx, cy)].push_back(i);
    }

    for(uint32_t i = 0; i < uint32_t(m_agents.size()); ++i)
    {
        const math::Vector pos = m_agents[i].body->GetPosition();
        const int32_t cx = int32_t(std::floor(pos.x * inv_cell));
        const int32_t cy = int32_t(std::floor(pos.y * inv_cell));

        math::Vector separation = math::ZeroVec;

        for(int32_t dx = -1; dx <= 1; ++dx)
        for(int32_t dy = -1; dy <= 1; ++dy)
        {
            const auto it = m_grid.find(CellKey(cx + dx, cy + dy));
            if(it == m_grid.end())
                continue;

            for(uint32_t j : it->second)
            {
                if(j == i)
                    continue;

                const math::Vector delta = pos - m_agents[j].body->GetPosition();
                const float dist_sq = math::LengthSquared(delta);
                if(dist_sq > 0.0f && dist_sq < math::Square(tweak_values::radius))
                {
                    const float dist = std::sqrt(dist_sq);
                    separation += (delta / dist) * (1.0f - dist / tweak_values::radius);
                }
            }
        }

        m_agents[i].separation = separation * tweak_values::strength;
    }
}
