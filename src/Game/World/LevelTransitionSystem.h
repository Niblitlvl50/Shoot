
#pragma once

#include "MonoFwd.h"
#include "Math/Vector.h"
#include "IGameSystem.h"

#include <cstdint>
#include <string>
#include <unordered_map>

namespace game
{
    // Moves the players to another level when a level_exit's trigger fires. The screen fades out
    // and the zone quits, the next level is picked by name instead of from the level's transitions.
    // The exit's entry tag survives the switch, and players spawn at an entity with the same tag
    // (from its tag component) in the next level.
    class LevelTransitionSystem : public mono::IGameSystem
    {
    public:

        LevelTransitionSystem(
            mono::TriggerSystem* trigger_system,
            mono::RenderSystem* render_system,
            mono::TransformSystem* transform_system,
            mono::IEntityManager* entity_manager,
            mono::EventHandler* event_handler);

        void AllocateLevelExit(uint32_t entity_id);
        void ReleaseLevelExit(uint32_t entity_id);
        void SetLevelExitData(uint32_t entity_id, uint32_t trigger_hash, const std::string& target_level, const std::string& entry_tag);

        // Where to spawn the players, the first entity with the tag the last level was left through,
        // or default_spawn_point if there's none. Uses up the tag.
        math::Vector GetPlayerSpawnPoint(const math::Vector& default_spawn_point);

        const char* Name() const override;
        void Reset() override;
        void Update(const mono::UpdateContext& update_context) override;

    private:

        void ExitLevel(uint32_t entity_id);

        mono::TriggerSystem* m_trigger_system;
        mono::RenderSystem* m_render_system;
        mono::TransformSystem* m_transform_system;
        mono::IEntityManager* m_entity_manager;
        mono::EventHandler* m_event_handler;

        struct LevelExit
        {
            uint32_t trigger_hash;
            uint32_t trigger_handle;
            std::string target_level;
            std::string entry_tag;
        };
        std::unordered_map<uint32_t, LevelExit> m_level_exits;

        bool m_exiting;

        // Kept between levels.
        std::string m_pending_entry_tag;
    };
}
