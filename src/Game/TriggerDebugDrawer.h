
#pragma once

#include "MonoFwd.h"
#include "Rendering/IDrawable.h"

namespace game
{
    class TriggerDebugDrawer : public mono::IDrawable
    {
    public:

        TriggerDebugDrawer(const bool& enabled, const class TriggerSystem* trigger_system, const mono::TransformSystem* transform_system);
        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

    private:

        const bool& m_enabled;
        const TriggerSystem* m_trigger_system;
        const mono::TransformSystem* m_transform_system;
    };
}
