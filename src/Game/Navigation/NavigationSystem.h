
#pragma once

#include "IGameSystem.h"
#include "NavMesh.h"
#include "NavmeshData.h"
#include "Physics/PhysicsFwd.h"

namespace game
{
    struct RecentPath
    {
        uint32_t timestamp;
        float time_ms;
        int nodes_evaluated;
        std::vector<math::Vector> points;
    };

    struct FindPathResult
    {
        AStarResult result;
        std::vector<math::Vector> nav_points;
    };

    class NavigationSystem : public mono::IGameSystem
    {
    public:

        NavigationSystem();
        const char* Name() const override;
        void Reset() override;
        void Update(const mono::UpdateContext& update_context) override;
        void Sync() override;

        void SetupNavmesh(const math::Vector& start, const math::Vector& end, float density, mono::PhysicsSpace* physics_space);
        const NavmeshContext* GetNavmeshContext() const;
        FindPathResult FindPath(const math::Vector& start, const math::Vector& end);
        const std::vector<RecentPath>& GetRecentPaths() const;
        int GetNumFindPath() const;

    private:

        NavmeshContext m_navmesh;
        uint32_t m_timestamp;

        int m_current_path_index;
        std::vector<RecentPath> m_recent_paths;

        int m_findpath_this_frame;
    };
}
