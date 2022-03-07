
#pragma once

#include "MonoFwd.h"
#include "Math/Vector.h"

namespace editor
{
    class CameraTool
    {
    public:

        CameraTool(mono::ICamera* camera);

        bool IsActive() const;
        void HandleMouseDown(const math::Vector& screen_position);
        void HandleMouseUp(const math::Vector& screen_position);
        void HandleMousePosition(const math::Vector& screen_position);

        void HandleMouseWheel(float x, float y);
        void HandleMultiGesture(const math::Vector& screen_position, float distance);

        void MoveCamera(const math::Vector& delta);

    private:
    
        mono::ICamera* m_camera;
        bool m_translate;
        math::Vector m_last_position;
    };
}
