
#pragma once

#include "MonoFwd.h"
#include "Math/Vector.h"
#include "IGameSystem.h"

#include <unordered_map>

namespace game
{
    class TeleportSystem : public mono::IGameSystem
    {
    public:

        TeleportSystem(
            class CameraSystem* camera_system,
            class TriggerSystem* trigger_system,
            mono::RenderSystem* render_system,
            mono::TransformSystem* transform_system);

        const char* Name() const override;
        void Update(const mono::UpdateContext& update_context) override;

        void AllocateTeleportPlayer(uint32_t entity_id);
        void ReleaseTeleportPlayer(uint32_t entity_id);
        void UpdateTeleportPlayer(uint32_t entity_id, uint32_t trigger_hash);

        void TeleportPlayers(const math::Vector& world_position);

    private:

        void HandleTeleport(uint32_t entity_id);

        class CameraSystem* m_camera_system;
        class TriggerSystem* m_trigger_system;
        mono::RenderSystem* m_render_system;
        mono::TransformSystem* m_transform_system;

        struct TeleportInfo
        {
            uint32_t trigger_hash;
            uint32_t trigger_handle;
        };
        std::unordered_map<uint32_t, TeleportInfo> m_teleport_infos;
        math::Vector m_saved_teleport_position;
    };
}
