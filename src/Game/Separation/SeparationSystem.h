
#pragma once

#include "IGameSystem.h"
#include "Physics/PhysicsFwd.h"
#include "Math/Vector.h"

#include <cstdint>
#include <unordered_map>
#include <vector>

namespace game
{
    class SeparationSystem : public mono::IGameSystem
    {
    public:
        void Register(uint32_t entity_id, mono::IBody* body);
        void Unregister(uint32_t entity_id);

        math::Vector GetSeparation(uint32_t entity_id) const;

        const char* Name() const override;
        void Update(const mono::UpdateContext& update_context) override;

    private:

        static uint64_t CellKey(int32_t cx, int32_t cy);

        struct Agent
        {
            uint32_t entity_id;
            mono::IBody* body;
            math::Vector separation;
        };

        std::vector<Agent> m_agents;
        std::unordered_map<uint64_t, std::vector<uint32_t>> m_grid;
    };
}
