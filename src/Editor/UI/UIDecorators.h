
#pragma once

#include <cstdint>

struct Component;

namespace editor
{
    struct UIContext;
    bool AreaEmitterFooter(const UIContext& ui_context, uint32_t component_index, Component& component);
    bool TexturedPolygonFooter(const UIContext& ui_context, uint32_t component_index, Component& component);
}
