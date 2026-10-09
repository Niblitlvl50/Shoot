
#pragma once

#include "MonoFwd.h"
#include "Rendering/IDrawable.h"

namespace game
{
    // Shows a label above cargo that's waiting to be loaded when a player gets close, telling
    // what it is, where it's going and how much room it takes in a train car.
    class TrainZoneSystemDrawer : public mono::IDrawable
    {
    public:

        TrainZoneSystemDrawer(
            const class TrainZoneSystem* train_zone_system,
            const mono::TransformSystem* transform_system);

        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

    private:

        const TrainZoneSystem* m_train_zone_system;
        const mono::TransformSystem* m_transform_system;
    };
}
