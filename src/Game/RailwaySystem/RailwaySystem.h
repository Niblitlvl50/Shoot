
#pragma once

#include "MonoFwd.h"
#include "Math/MathFwd.h"
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

        // Emitted whenever the active branch changes (see ToggleSwitch).
        uint32_t primary_trigger_hash = 0;
        uint32_t alt_trigger_hash = 0;
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
            uint32_t trigger_hash,
            uint32_t primary_trigger_hash,
            uint32_t alt_trigger_hash);

        // Flips which branch is active. Also called automatically when the switch's
        // trigger fires (e.g. from a level-placed interaction switch).
        void ToggleSwitch(uint32_t switch_entity_id);

        // Tries to flip whichever switch sits at the end of `entity_id`'s current track in
        // the direction it's currently being driven (current throttle sign; treated as
        // forward if zero) - e.g. hook this up to a horn button so honking near a junction
        // flips it ahead of arriving. Returns false if there's no switch there.
        bool ToggleSwitchAhead(uint32_t entity_id);

        // Whether a switch at the end of entity_id's current track leads on to another track, at the
        // end it reaches going forward (increasing path position) or backward. It's the track the
        // train would be handed onto with the switches as they are now.
        bool HasTrackAhead(uint32_t entity_id, bool forward) const;

        // The switch at the end of entity_id's current track going forward or backward, but only when
        // the train is on that switch's trunk, the side where the switch decides which branch it takes.
        // INVALID_ID otherwise.
        uint32_t FindBranchingSwitchAhead(uint32_t entity_id, bool forward) const;

        RailwayStationComponent* AllocateStation(uint32_t entity_id);
        void ReleaseStation(uint32_t entity_id);
        void SetStationData(uint32_t entity_id, const std::string& name);

        template <typename T>
        inline void ForEachStation(T&& callable) const
        {
            for(const auto& station_pair : m_stations)
                callable(station_pair.first);
        }

    private:

        const char* Name() const override;
        void Update(const mono::UpdateContext& update_context) override;

        void TryHandOff(uint32_t train_entity_id);

        // The track a train on current_track_id continues onto at the switch near endpoint_position,
        // or INVALID_ID when there's no switch there or it leads nowhere.
        uint32_t FindNextTrack(uint32_t current_track_id, const math::Vector& endpoint_position) const;
        uint32_t FindSwitchNearPosition(const math::Vector& position) const;
        void DrawDebugInfo() const;

        mono::SystemContext* m_system_context;
        std::unordered_map<uint32_t, RailwaySwitchComponent> m_switches;
        std::unordered_map<uint32_t, RailwayStationComponent> m_stations;
    };
}
