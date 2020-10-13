
#pragma once

#include "MonoFwd.h"
#include "Math/Vector.h"

namespace System
{
    class IWindow;
}

namespace editor
{
    class CameraTool
    {
    public:

        CameraTool(mono::ICamera* camera, const System::IWindow* window);

        bool IsActive() const;
        void HandleMouseDown(const math::Vector& screen_position);
        void HandleMouseUp(const math::Vector& screen_position);
        void HandleMousePosition(const math::Vector& screen_position);

        void HandleMouseWheel(float x, float y);
        void HandleMultiGesture(const math::Vector& screen_position, float distance);

    private:
    
        mono::ICamera* m_camera;
        const System::IWindow* m_window;

        bool m_translate;
        math::Vector m_last_position;
    };
}
