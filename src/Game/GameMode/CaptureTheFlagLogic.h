
#pragma once

#include "IUpdatable.h"
#include "Math/Vector.h"

#include "CaptureTheFlagScore.h"

#include <vector>
#include <cstdint>

class IEntityManager;

namespace mono
{
    class TransformSystem;
    class SpriteSystem;
}

namespace game
{
    class PlayerDaemon;
    class DamageSystem;

    class CaptureTheFlagLogic : public mono::IUpdatable
    {
    public:
        CaptureTheFlagLogic(
            const std::vector<uint32_t>& flags,
            mono::TransformSystem* transform_system,
            mono::SpriteSystem* sprite_system,
            game::DamageSystem* damage_system,
            const PlayerDaemon* player_daemon,
            IEntityManager* entity_manager);
        
        void doUpdate(const mono::UpdateContext& update_context) override;

        void Start();
        void End();
        const CaptureTheFlagScore& Score() const;
        void ResetScore();
        void DropFlag(uint32_t flag_owner_entity_id);

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
            uint32_t callback_handle;

            math::Vector spawn_position;
            FlagState state;
            int score;
        };

        void CheckForPickup(const std::vector<uint32_t>& player_ids, FlagData& flag);
        void CheckForFlagDrop(FlagData& flag);
        
        std::vector<FlagData> m_flags;

        mono::TransformSystem* m_transform_system;
        game::DamageSystem* m_damage_system;
        const PlayerDaemon* m_player_daemon;
        IEntityManager* m_entity_manager;

        CaptureTheFlagScore m_score;
    };
}
