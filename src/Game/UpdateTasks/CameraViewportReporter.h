
#pragma once

#include "MonoFwd.h"
#include "IUpdatable.h"

namespace game
{
    class CameraViewportReporter : public mono::IUpdatable
    {
    public:
        CameraViewportReporter(const mono::ICamera* camera);
        void doUpdate(const mono::UpdateContext& update_context) override;

    private:
        const mono::ICamera* m_camera;
    };
}
