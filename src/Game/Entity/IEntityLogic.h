
#pragma once

#include "IUpdatable.h"
#include "Rendering/IDrawable.h"
#include "Math/Quad.h"

namespace game
{
    class IEntityLogic : public mono::IUpdatable, public mono::IDrawable
    {
    public:

        void Update(const mono::UpdateContext& update_context) override
        { }

        void Draw(mono::IRenderer& renderer) const override
        { }

        math::Quad BoundingBox() const override
        {
            return math::InfQuad;
        }

        virtual void DrawDebugInfo(class IDebugDrawer* debug_drawer) const
        { }

        virtual const char* GetDebugCategory() const
        {
            return "Unknown";
        }
    };
}
