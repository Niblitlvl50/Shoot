
#pragma once

#include "Rendering/IDrawable.h"
#include <string>

namespace animator
{
    class InterfaceDrawer : public mono::IDrawable
    {
    public:

        InterfaceDrawer(struct UIContext& context);
        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

        struct UIContext& m_context;
    };

    struct SpritePickerResult
    {
        bool changed;
        std::string new_value;
    };
    SpritePickerResult DrawSpritePicker(const char* name, const std::string& current_value, const UIContext& ui_context);
}
