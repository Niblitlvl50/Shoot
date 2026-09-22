
#pragma once

#include "MonoFwd.h"
#include "MonoPtrFwd.h"
#include "Physics/PhysicsFwd.h"
#include "Math/Vector.h"

#include <cstdint>
#include <vector>

namespace game
{
    struct PathResult
    {
        float distance_to_target;
        bool is_stuck;
    };

    struct PathDebugData
    {
        bool has_path = false;
        math::Vector target_position;
        const std::vector<math::Vector>* path_points = nullptr;
    };

    class PathBehaviour
    {
    public:

        PathBehaviour();
        PathBehaviour(mono::IBody* body, mono::IPathPtr path);
        PathBehaviour(PathBehaviour&& other) noexcept;
        PathBehaviour& operator=(PathBehaviour&& other) noexcept;
        ~PathBehaviour();

        void Init(mono::IBody* body);
        void SetPath(const mono::IPathPtr path);
        void SetTrackingSpeed(float meter_per_second);
        void SetPingPong(bool ping_pong);
        void SetLoop(bool loop);
        void SetApplyRotation(bool apply_rotation);
        void SetOffset(const math::Vector& offset);
        void SetPaused(bool paused);

        // When enabled, position along the path is driven by SetThrottle() calls each frame
        // instead of automatically advancing (and ignores ping-pong/loop).
        void SetManualControl(bool manual_control);
        void SetThrottle(float throttle);
        float GetThrottle() const;

        // Used by the railway system to hand a manually-controlled entity off from one
        // path to another (e.g. at a switch) without losing its position along the track.
        void SetCurrentPosition(float position);
        void TeleportToPosition(float position);
        float GetCurrentPosition() const;
        float GetPathLength() const;
        const std::vector<math::Vector>* GetPathPoints() const;

        PathResult Run(float delta_s);
        PathDebugData GetDebugData() const;

    private:

        mono::IPathPtr m_path;
        mono::IBody* m_entity_body;

        float m_current_position = 0.0f;
        float m_meter_per_second = 1.0f;
        float m_direction = 1.0f;
        bool m_ping_pong = false;
        bool m_loop = false;
        bool m_apply_rotation = false;
        bool m_paused = false;
        bool m_manual_control = false;
        float m_throttle = 0.0f;
        math::Vector m_move_velocity;
        math::Vector m_offset;
    };
}
