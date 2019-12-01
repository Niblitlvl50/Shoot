
#pragma once

#include "IUpdatable.h"
#include "Math/Vector.h"

#include <vector>
#include <cstdint>

namespace mono
{
    class TransformSystem;
}

namespace game
{
    class PlayerDaemon;

    struct FlagDropzonePair
    {
        uint32_t flag_entity_id;
        uint32_t dropzone_entity_id;
    };

    class CaptureTheFlagLogic : public mono::IUpdatable
    {
    public:
        CaptureTheFlagLogic(
            const std::vector<FlagDropzonePair>& flags, mono::TransformSystem* transform_system, const PlayerDaemon* player_daemon);
        void doUpdate(const mono::UpdateContext& update_context) override;

    private:

        enum class FlagState
        {
            NONE,
            PICKED_UP
        };

        struct FlagData
        {
            uint32_t flag_entity_id;
            uint32_t dropzone_entity_id;
            uint32_t owning_entity_id;
            math::Vector spawn_position;
            FlagState state;
        };

        void CheckForPickup(const std::vector<uint32_t>& player_ids, FlagData& flag) const;
        void CheckForFlagDrop(FlagData& flag) const;
        
        std::vector<FlagData> m_flags;
        mono::TransformSystem* m_transform_system;
        const PlayerDaemon* m_player_daemon;
    };
}
