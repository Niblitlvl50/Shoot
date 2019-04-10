
#pragma once

#include "Math/MathFwd.h"
#include <cstdint>

namespace editor
{
    class ITool
    {
    public:

        virtual ~ITool() = default;
        virtual void Begin() = 0;
        virtual void End() = 0;
        virtual bool IsActive() const = 0;
        virtual void HandleContextMenu(int menu_index) = 0;
        virtual void HandleMouseDown(const math::Vector& world_pos, uint32_t entity_id) = 0;
        virtual void HandleMouseUp(const math::Vector& world_pos) = 0;
        virtual void HandleMousePosition(const math::Vector& world_pos) = 0;
        virtual void UpdateModifierState(bool ctrl, bool shift, bool alt) = 0;
    };
}
