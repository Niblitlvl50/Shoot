
#pragma once

#include "MonoFwd.h"
#include "IGameSystem.h"
#include "Behaviour/PathBehaviour.h"
#include "Math/Vector.h"

#include <cstdint>
#include <unordered_map>

namespace game
{
    struct PathFollowerComponent
    {
        PathBehaviour behaviour;
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
            const math::Vector& offset);

        void SetPaused(uint32_t entity_id, bool paused);
        void SetSpeed(uint32_t entity_id, float speed);
        void SetOffset(uint32_t entity_id, const math::Vector& offset);

    private:

        const char* Name() const override;
        void Update(const mono::UpdateContext& update_context) override;

        mono::SystemContext* m_system_context;
        std::unordered_map<uint32_t, PathFollowerComponent> m_components;
    };
}
