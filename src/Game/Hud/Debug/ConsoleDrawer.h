
#pragma once

#include "Rendering/IDrawable.h"

#include <vector>
#include <string>

namespace game
{
    class ConsoleDrawer : public mono::IDrawable
    {
    public:

        void AddText(const std::string& text, uint32_t life_time_ms);

        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

    private:

        struct TextItem
        {
            std::string text;
            uint32_t life;
        };

        mutable std::vector<TextItem> m_text_items;
    };
}
