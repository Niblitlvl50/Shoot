
#pragma once

#include "Rendering/IDrawable.h"
#include "Math/Matrix.h"

#include <vector>

namespace game
{
    class UIElement
    {
    public:

        virtual void Draw(mono::IRenderer& renderer) const = 0;
    };

    class UIOverlayDrawer : public mono::IDrawable
    {
    public:

        UIOverlayDrawer();
        ~UIOverlayDrawer();
        
        virtual void doDraw(mono::IRenderer& renderer) const;
        virtual math::Quad BoundingBox() const;

        int AddElement(std::unique_ptr<UIElement> element);
        //void RemoveElement(int id);

        math::Matrix m_projection;
        std::vector<std::unique_ptr<UIElement>> m_ui_elements; 
    };
}
