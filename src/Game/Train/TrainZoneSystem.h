
#pragma once

#include "MonoFwd.h"
#include "IGameSystem.h"
#include "Math/Vector.h"
#include "System/Audio.h"

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace game
{
    // Something that can be loaded into a train car. Its value is how much of a car's
    // capacity it takes up, and it can only be dropped off at a zone with a matching destination.
    struct CargoComponent
    {
        // Shown to the player, the entity's name is used when empty.
        std::string name;
        int value = 0;
        std::string destination;
        bool loaded = false;
        bool delivered = false;

        // Collision masks of the entity's shapes, saved while it's hidden in a car.
        std::vector<uint32_t> saved_collision_masks;
    };

    struct TrainZoneComponent
    {
        math::Vector size;
        uint32_t trigger_hash;

        // Seconds it takes to load or unload each cargo, zero moves everything at once.
        float transfer_duration_s;

        // Only used by drop off zones, cargo is dropped off where its destination matches.
        std::string destination;
    };

    // Moves cargo in and out of train cars. A train car that is stopped inside a loading zone
    // picks up every undelivered cargo entity inside the same zone that fits in it. A stopped
    // car inside a drop off zone unloads the cargo whose destination matches the zone's.
    // Loaded cargo is hidden, its sprites are disabled and its shapes stop colliding, until
    // it's dropped off at the zone's position.
    class TrainZoneSystem : public mono::IGameSystem
    {
    public:

        TrainZoneSystem(
            mono::TransformSystem* transform_system,
            mono::PhysicsSystem* physics_system,
            mono::ParticleSystem* particle_system,
            mono::IEntityManager* entity_manager,
            mono::TriggerSystem* trigger_system,
            class TrainCarSystem* train_car_system);
        ~TrainZoneSystem();

        void AllocateCargo(uint32_t entity_id);
        void ReleaseCargo(uint32_t entity_id);
        void SetCargoData(uint32_t entity_id, const std::string& name, int value, const std::string& destination);

        void AllocateLoadingZone(uint32_t entity_id);
        void ReleaseLoadingZone(uint32_t entity_id);
        void SetLoadingZoneData(uint32_t entity_id, const math::Vector& size, float transfer_duration_s, uint32_t trigger_hash);

        void AllocateDropOffZone(uint32_t entity_id);
        void ReleaseDropOffZone(uint32_t entity_id);
        void SetDropOffZoneData(
            uint32_t entity_id, const math::Vector& size, float transfer_duration_s, const std::string& destination, uint32_t trigger_hash);

        template <typename T>
        inline void ForEachCargo(T&& callable) const
        {
            for(const auto& [entity_id, cargo] : m_cargo)
                callable(entity_id, cargo);
        }

        // Loading only happens while this is held, letting go before a cargo is loaded aborts it.
        void SetLoadingHeld(bool held);

        struct LoadingProgress
        {
            uint32_t car_entity_id;
            float fraction;
        };
        std::vector<LoadingProgress> GetLoadingProgress() const;

        const char* Name() const override;
        void Begin() override;
        void Reset() override;
        void Update(const mono::UpdateContext& update_context) override;

    private:

        void UpdateLoadingZone(
            uint32_t zone_entity_id, const TrainZoneComponent& zone, const std::vector<uint32_t>& stopped_cars, float delta_s);
        void UpdateDropOffZone(
            uint32_t zone_entity_id, const TrainZoneComponent& zone, const std::vector<uint32_t>& stopped_cars, float delta_s);

        // How many cargo the car may move this frame, ticking the car's transfer timer for the zone.
        // Only called while the car is stopped in the zone with something to move.
        int TickTransferTimer(uint32_t car_id, uint32_t zone_entity_id, const TrainZoneComponent& zone, bool is_loading, float delta_s);
        bool IsInsideZone(uint32_t zone_entity_id, const TrainZoneComponent& zone, uint32_t entity_id) const;

        void HideCargo(uint32_t cargo_entity_id, CargoComponent& cargo);
        void ShowCargo(uint32_t cargo_entity_id, CargoComponent& cargo, const math::Vector& position);

        mono::TransformSystem* m_transform_system;
        mono::PhysicsSystem* m_physics_system;
        mono::ParticleSystem* m_particle_system;
        mono::TriggerSystem* m_trigger_system;
        mono::IEntityManager* m_entity_manager;
        TrainCarSystem* m_train_car_system;

        audio::ISoundPtr m_loading_sound;
        audio::ISoundPtr m_unloading_sound;
        std::unique_ptr<class CargoTransferEffect> m_transfer_effect;

        bool m_loading_held;

        std::unordered_map<uint32_t, CargoComponent> m_cargo;
        std::unordered_map<uint32_t, TrainZoneComponent> m_loading_zones;
        std::unordered_map<uint32_t, TrainZoneComponent> m_drop_off_zones;

        struct TransferTimer
        {
            uint32_t car_entity_id = 0;
            float elapsed_s = 0.0f;
            float duration_s = 0.0f;
            bool is_loading = false;
            bool ticked = false;
        };

        // Keyed on car and zone together, so a car in a loading and a drop off zone at once has
        // one timer for each. Reset when the car leaves the zone, starts moving or runs out of
        // cargo to move there.
        std::unordered_map<uint64_t, TransferTimer> m_transfer_timers;
    };
}
