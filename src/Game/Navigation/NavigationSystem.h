
#pragma once

#include "IGameSystem.h"
#include "NavmeshData.h"
#include "Physics/PhysicsFwd.h"

namespace game
{
    struct RecentPath
    {
        uint32_t timestamp;
        std::vector<math::Vector> points;
    };

    class NavigationSystem : public mono::IGameSystem
    {
    public:

        NavigationSystem();
        const char* Name() const override;
        void Reset() override;
        void Update(const mono::UpdateContext& update_context) override;

        void SetupNavmesh(const math::Vector& start, const math::Vector& end, float density, mono::PhysicsSpace* physics_space);
        const NavmeshContext* GetNavmeshContext() const;
        const std::vector<math::Vector>& FindPath(const math::Vector& start, const math::Vector& end);
        const std::vector<RecentPath>& GetRecentPaths() const;

    private:

        NavmeshContext m_navmesh;
        uint32_t m_timestamp;

        int m_current_path_index;
        std::vector<RecentPath> m_recent_paths;
    };
}
