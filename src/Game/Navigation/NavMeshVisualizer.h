
#pragma once

#include "MonoFwd.h"
#include "EventHandler/EventToken.h"
#include "Events/EventFwd.h"
#include "Rendering/IDrawable.h"

#include <vector>

namespace game
{
    struct NavmeshContext;

    class NavmeshVisualizer : public mono::IDrawable
    {
    public:

        NavmeshVisualizer(const NavmeshContext& context, mono::EventHandler& event_handler);
        ~NavmeshVisualizer();

    private:

        void doDraw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

        bool OnMouseUp(const event::MouseUpEvent& event);        

        const NavmeshContext& m_navmesh_context;
        mono::EventHandler& m_event_handler;

        std::vector<math::Vector> m_edges;
        std::vector<math::Vector> m_navigation_points;

        mono::EventToken<event::MouseUpEvent> m_mouse_up_token;
        
        int m_start = 0;
        int m_end = 0;
    };
}
