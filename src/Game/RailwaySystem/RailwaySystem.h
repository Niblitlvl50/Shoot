
#pragma once

#include "MonoFwd.h"
#include "IGameSystem.h"
#include "EntitySystem/Entity.h"

#include <cstdint>
#include <string>
#include <unordered_map>

namespace game
{
    struct RailwaySwitchComponent
    {
        uint32_t trunk_track_entity_id = mono::INVALID_ID;
        uint32_t primary_track_entity_id = mono::INVALID_ID;
        uint32_t alt_track_entity_id = mono::INVALID_ID;
        bool use_alt_branch = false;

        uint32_t trigger_hash = 0;
        uint32_t trigger_callback_id = mono::INVALID_ID;
    };

    struct RailwayStationComponent
    {
        std::string name;
    };

    // Tracks the network of railway switches and stations. A switch connects a single
    // trunk track to two branch tracks (primary/alt); when a manually-driven path_follower
    // entity (e.g. the player's train) reaches either end of a track that meets a switch,
    // it's automatically handed off onto whichever track continues from there - the switch's
    // active branch when coming from the trunk, or back onto the trunk when coming from
    // either branch.
    class RailwaySystem : public mono::IGameSystem
    {
    public:

        RailwaySystem(mono::SystemContext* system_context);

        RailwaySwitchComponent* AllocateSwitch(uint32_t entity_id);
        void ReleaseSwitch(uint32_t entity_id);
        void SetSwitchData(
            uint32_t entity_id,
            uint32_t trunk_track_reference,
            uint32_t primary_track_reference,
            uint32_t alt_track_reference,
            bool use_alt_branch,
            uint32_t trigger_hash);

        // Flips which branch is active. Also called automatically when the switch's
        // trigger fires (e.g. from a level-placed interaction switch).
        void ToggleSwitch(uint32_t switch_entity_id);

        RailwayStationComponent* AllocateStation(uint32_t entity_id);
        void ReleaseStation(uint32_t entity_id);
        void SetStationData(uint32_t entity_id, const std::string& name);

    private:

        const char* Name() const override;
        void Update(const mono::UpdateContext& update_context) override;

        void TryHandOff(uint32_t train_entity_id);
        void DrawDebugInfo() const;

        mono::SystemContext* m_system_context;
        std::unordered_map<uint32_t, RailwaySwitchComponent> m_switches;
        std::unordered_map<uint32_t, RailwayStationComponent> m_stations;
    };
}
