
#pragma once

#include "MonoFwd.h"
#include "MonoPtrFwd.h"
#include "IGameSystem.h"
#include "EntitySystem/Entity.h"
#include "Behaviour/PathBehaviour.h"
#include "Math/Vector.h"

#include <cstdint>
#include <unordered_map>
#include <vector>

namespace game
{
    struct PathFollowerComponent
    {
        PathBehaviour behaviour;
        uint32_t current_path_entity_id = mono::INVALID_ID;

        // The referenced path entity's own components may not have been set yet (entity
        // creation order isn't guaranteed), so the actual lookup/bake is deferred to Sync().
        uint32_t pending_path_entity_reference = mono::INVALID_ID;
        bool needs_path_resolve = false;
        float initial_position = 0.0f;
    };

    // Drives an entity's physics body along another entity's path, configured entirely
    // through PATH_FOLLOWER_COMPONENT data rather than a bespoke controller.
    class PathFollowerSystem : public mono::IGameSystem
    {
    public:

        PathFollowerSystem(mono::SystemContext* system_context);

        PathFollowerComponent* AllocatePathFollower(uint32_t entity_id);
        void ReleasePathFollower(uint32_t entity_id);

        void SetPathFollowerData(
            uint32_t entity_id,
            uint32_t path_entity_reference,
            float speed,
            bool loop,
            bool ping_pong,
            bool apply_rotation,
            const math::Vector& offset,
            bool manual_control);

        void SetPathReference(uint32_t entity_id, uint32_t path_entity_reference, float initial_position);


        void SetPaused(uint32_t entity_id, bool paused);
        void SetSpeed(uint32_t entity_id, float speed);
        void SetOffset(uint32_t entity_id, const math::Vector& offset);
        void SetCurrentPosition(uint32_t entity_id, float position);

        // Only meaningful when the component's manual_control flag is set; drives the
        // entity's position along the path directly, ignoring speed/loop/ping-pong.
        void SetThrottle(uint32_t entity_id, float throttle);
        float GetThrottle(uint32_t entity_id) const;

        uint32_t GetCurrentPathEntity(uint32_t entity_id) const;
        bool IsAtPathStart(uint32_t entity_id) const;
        bool IsAtPathEnd(uint32_t entity_id) const;
        const std::vector<math::Vector>* GetPathPoints(uint32_t entity_id) const;

        // Radians of turn per meter at the entity's current position on its path; 0 if the
        // entity has no path (or the path is a straight line), larger magnitude for tighter bends.
        float GetCurvature(uint32_t entity_id) const;

        // Hands a manually-controlled entity off onto a different track, entering it at
        // whichever end lies closest to `enter_at_world_position` (used at railway switches).
        // `entering_forward` says which way the entity was travelling when it hit the switch:
        // the new track is oriented so continuing with that same throttle direction keeps
        // moving away from the entry point (forward entries land at position 0, backward
        // entries at the far end), instead of getting stuck at a clamped boundary and
        // immediately handing back off the way it came.
        // Returns false if the entity, the new path, or a matching endpoint can't be found.
        bool SwitchToPathEntity(uint32_t entity_id, uint32_t new_path_entity_id, const math::Vector& enter_at_world_position, bool entering_forward);

        template <typename T>
        void ForEach(T&& callback) const
        {
            for(const auto& entity_component_pair : m_components)
                callback(entity_component_pair.first);
        }

    private:

        const char* Name() const override;
        void Update(const mono::UpdateContext& update_context) override;
        void Sync() override;

        mono::IPathPtr BakePath(uint32_t path_entity_id) const;

        mono::SystemContext* m_system_context;
        std::unordered_map<uint32_t, PathFollowerComponent> m_components;
    };
}
