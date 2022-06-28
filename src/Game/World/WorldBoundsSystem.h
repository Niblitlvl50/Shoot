
#pragma once

#include "IGameSystem.h"
#include "Math/Vector.h"

#include <vector>
#include <string>

namespace game
{
    class WorldBoundsSystem : public mono::IGameSystem
    {
    public:

        WorldBoundsSystem();

        void AddPolygon(const std::vector<math::Vector>& vertices, const std::string& texture_file);

        uint32_t Id() const override;
        const char* Name() const override;
        void Update(const mono::UpdateContext& update_context) override;
    };
}
