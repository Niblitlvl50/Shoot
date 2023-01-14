
#pragma once

#include "MonoFwd.h"
#include "MonoPtrFwd.h"
#include "EventHandler/EventToken.h"
#include "Events/EventFwd.h"
#include "Rendering/IDrawable.h"

#include <vector>

namespace game
{
    class NavmeshVisualizer : public mono::IDrawable
    {
    public:

        NavmeshVisualizer(const class NavigationSystem* navigation_system, mono::EventHandler& event_handler);
        ~NavmeshVisualizer();

    private:

        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

        mono::EventResult OnMouseUp(const event::MouseUpEvent& event);
        mono::EventResult OnMouseMove(const event::MouseMotionEvent& event);

        const NavigationSystem* m_navigation_system;
        mono::EventHandler& m_event_handler;

        std::vector<math::Vector> m_edges;

        mono::IPathPtr m_path;

        mono::EventToken<event::MouseUpEvent> m_mouse_up_token;
        mono::EventToken<event::MouseMotionEvent> m_mouse_motion_token;
        
        int m_start = 0;
        int m_end = 0;
        float m_at_length = 0.0f;
    };
}
