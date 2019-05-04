
#pragma once

#include "MonoPtrFwd.h"
#include "IUpdatable.h"

namespace game
{
    class CameraViewportReporter : public mono::IUpdatable
    {
    public:
        CameraViewportReporter(const mono::ICameraPtr& camera);
        void doUpdate(const mono::UpdateContext& update_context) override;

    private:
        const mono::ICameraPtr& m_camera;
    };
}
