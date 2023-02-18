
#pragma once

#include "MonoFwd.h"
#include "Math/MathFwd.h"
#include "Math/Vector.h"
#include "IGameSystem.h"
#include "Rendering/IDrawable.h"
#include "StateMachine.h"

#include <unordered_map>

namespace game
{
    class TeleportSystem : public mono::IGameSystem
    {
    public:

        TeleportSystem(
            class CameraSystem* camera_system, class TriggerSystem* trigger_system, mono::TransformSystem* transform_system);

        const char* Name() const override;
        void Update(const mono::UpdateContext& update_context) override;

        void AllocateTeleportPlayer(uint32_t entity_id);
        void ReleaseTeleportPlayer(uint32_t entity_id);
        void UpdateTeleportPlayer(uint32_t entity_id, uint32_t trigger_hash);

        void TeleportPlayers(const math::Vector& world_position);

        bool ShouldApplyFadeAlpha() const;
        float GetFadeAlpha() const;

    private:

        void ToIdle();
        void Idle(const mono::UpdateContext& update_context);

        void ToFadeOut();
        void FadeOut(const mono::UpdateContext& update_context);

        void ToFadeIn();
        void FadeIn(const mono::UpdateContext& update_context);

        void HandleTeleport(uint32_t entity_id);

        class CameraSystem* m_camera_system;
        class TriggerSystem* m_trigger_system;
        mono::TransformSystem* m_transform_system;

        struct TeleportInfo
        {
            uint32_t trigger_hash;
            uint32_t trigger_handle;
        };
        std::unordered_map<uint32_t, TeleportInfo> m_teleport_infos;

        enum class States
        {
            IDLE,
            FADE_OUT,
            FADE_IN
        };

        using TeleportStateMachine = StateMachine<States, const mono::UpdateContext&>;
        TeleportStateMachine m_states;

        math::Vector m_saved_teleport_position;
        float m_fade_timer;
        float m_alpha;
    };

    class TeleportSystemDrawer : public mono::IDrawable
    {
    public:

        TeleportSystemDrawer(const TeleportSystem* teleport_system);
        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

        const TeleportSystem* m_teleport_system;
    };
}
