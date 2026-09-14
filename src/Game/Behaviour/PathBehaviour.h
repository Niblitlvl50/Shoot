
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
        ~PathBehaviour();

        void Init(mono::IBody* body);
        void SetPath(const mono::IPathPtr path);
        void SetTrackingSpeed(float meter_per_second);
        PathResult Run(float delta_s);
        PathDebugData GetDebugData() const;

    private:

        mono::IPathPtr m_path;
        mono::IBody* m_entity_body;

        float m_current_position = 0.0f;
        float m_meter_per_second = 1.0f;
        math::Vector m_move_velocity;
    };
}
