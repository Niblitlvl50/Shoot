
#pragma once

#include "Rendering/IDrawable.h"
#include "Rendering/RenderPtrFwd.h"

#include <vector>
#include <string>

namespace game
{
    class ConsoleDrawer : public mono::IDrawable
    {
    public:

        void AddText(const std::string& text);

        void doDraw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

    private:

        struct TextItem
        {
            std::string text;
            int life;
        };

        mutable std::vector<TextItem> m_text_items;
    };
}
