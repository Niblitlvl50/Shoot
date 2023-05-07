
#pragma once

#include "IGameSystem.h"

#include <cstdint>
#include <vector>

namespace game
{
    class WorldEntityTrackingSystem : public mono::IGameSystem
    {
    public:

        WorldEntityTrackingSystem();

        const char* Name() const override;
        void Update(const mono::UpdateContext& update_context) override;

        void TrackEntity(uint32_t entity_id);
        void ForgetEntity(uint32_t entity_id);

        const std::vector<uint32_t>& GetTrackedEntities() const;

    private:

        std::vector<uint32_t> m_entities_to_track;
    };
}
