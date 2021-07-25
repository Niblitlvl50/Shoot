
#pragma once

#include "ITool.h"
#include "Math/Vector.h"
#include <vector>

namespace editor
{
    class TranslateTool : public ITool
    {
    public:

        TranslateTool(class Editor* editor);

        virtual void Begin();
        virtual void End();
        virtual bool IsActive() const;
        virtual void HandleContextMenu(int menu_index);
        virtual void HandleMouseDown(const math::Vector& world_pos, uint32_t entity_id);
        virtual void HandleMouseUp(const math::Vector& world_pos);
        virtual void HandleMousePosition(const math::Vector& world_pos);
        virtual void UpdateModifierState(bool ctrl, bool shift, bool alt);

        void MoveObjects(const math::Vector& delta);

    private:

        Editor* m_editor;
        math::Vector m_begin_translate;
        std::vector<math::Vector> m_proxy_start_positions;
    };
}
