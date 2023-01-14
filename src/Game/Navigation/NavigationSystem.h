
#pragma once

#include "IGameSystem.h"
#include "NavmeshData.h"
#include "Physics/PhysicsFwd.h"

namespace game
{
    class NavigationSystem : public mono::IGameSystem
    {
    public:

        const char* Name() const override;
        void Reset() override;
        void Update(const mono::UpdateContext& update_context) override;

        void SetupNavmesh(const math::Vector& start, const math::Vector& end, float density, mono::PhysicsSpace* physics_space);
        const NavmeshContext* GetNavmeshContext() const;

    private:

        NavmeshContext m_navmesh;
    };
}
