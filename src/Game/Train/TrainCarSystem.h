
#pragma once

#include "MonoFwd.h"
#include "IGameSystem.h"
#include "EntitySystem/Entity.h"
#include "Math/Vector.h"
#include "System/Audio.h"

#include <cstdint>
#include <unordered_map>

namespace game
{
    struct TrainCarComponent
    {
        // The neighbour this car takes its speed from - set when the car couples onto
        // something (or at runtime via SetLeader()). A car's neighbours are its leader plus
        // every car that has it as their leader, at most one at each end.
        uint32_t leader_entity_id = mono::INVALID_ID;

        // This car's share of the gap to a coupled neighbour: the coupling points sit
        // coupling_distance / 2 from the car's centre, and two coupled cars are held
        // (a.coupling_distance + b.coupling_distance) / 2 apart, centre to centre.
        float coupling_distance = 1.5f;

        // True for the player-driven locomotive: its throttle always comes from player
        // input, never from this system, and it's never assigned a leader of its own - but
        // both its ends are ordinary coupling points for other cars.
        bool is_locomotive = false;

        // Counts down after this car is decoupled; while positive, it won't be considered
        // for auto-coupling, so it doesn't immediately re-attach to whatever it just left.
        float decouple_cooldown_s = 0.0f;
    };

    // Drives "train car" entities that couple to each other end to end. Each car has two
    // coupling points, one at each end along its track, and each can hold one neighbour.
    // The entity must have its own path_follower component (train_car declares it as a
    // dependency) with manual_control and apply_rotation enabled; this system only drives
    // its throttle each frame to match its leader's speed and hold the coupling gap. Since
    // each car is an independent, manually-controlled path_follower, RailwaySystem hands it
    // across switches exactly like a locomotive.
    //
    // All following is worked out in world space against the car's own track direction,
    // never by comparing arc-length positions between entities: after a switch hand-off two
    // entities can be on the same track entity with its points baked in opposite
    // directions, so their arc-length positions aren't comparable.
    //
    // Coupling is automatic: each frame, when a free coupling point of a car comes within
    // range of a free coupling point of a car in a different chain, the two chains join. The
    // car that isn't part of a locomotive-driven chain becomes the follower; if it already
    // has a leader, the leader links along its chain are reversed first so the new
    // coupling becomes the chain's head. Two locomotive-driven chains never couple.
    class TrainCarSystem : public mono::IGameSystem
    {
    public:

        TrainCarSystem(mono::SystemContext* system_context);

        TrainCarComponent* AllocateTrainCar(uint32_t entity_id);
        void ReleaseTrainCar(uint32_t entity_id);
        void SetTrainCarData(uint32_t entity_id, float coupling_distance, bool is_locomotive);

        // Couples this car to its leader at runtime (a live entity id). If the car isn't on
        // a track yet it's put on the leader's, at the point closest to where it is.
        void SetLeader(uint32_t entity_id, uint32_t leader_entity_id);

        // Detaches every car whose leader is currently `leader_entity_id` (e.g. call with
        // the locomotive's id to decouple whatever's directly attached to it). Anything
        // further along the chain keeps following the car next to it, so it stays linked.
        void Decouple(uint32_t leader_entity_id);

    private:

        const char* Name() const override;
        void Update(const mono::UpdateContext& update_context) override;
        void Sync() override;

        void TryAutoCouple();
        void DrawDebugInfo() const;

        struct CouplingPoint
        {
            math::Vector position;
            bool is_free;
        };

        // side is +1 for the end in the car's own increasing-path-position direction, -1 for
        // the other end.
        CouplingPoint GetCouplingPoint(uint32_t entity_id, float side) const;
        float GetCouplingDistance(uint32_t entity_id) const;

        // The car at the head of entity_id's chain (the one with no leader).
        uint32_t FindChainRoot(uint32_t entity_id) const;

        // Reverses the leader links from entity_id up to its chain's root, making entity_id
        // the new root.
        void MakeChainRoot(uint32_t entity_id);

        mono::SystemContext* m_system_context;
        std::unordered_map<uint32_t, TrainCarComponent> m_cars;

        audio::ISoundPtr m_couple_sound;
        audio::ISoundPtr m_decouple_sound;
    };
}
