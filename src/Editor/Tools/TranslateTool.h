
#pragma once

#include "MonoFwd.h"
#include "ITool.h"
#include "Math/Vector.h"
#include <memory>

namespace editor
{
    class Editor;

    class TranslateTool : public ITool
    {
    public:

        TranslateTool(Editor* editor);

        virtual void Begin();
        virtual void End();
        virtual bool IsActive() const;
        virtual void HandleContextMenu(int menu_index);
        virtual void HandleMouseDown(const math::Vector& world_pos, uint32_t entity_id);
        virtual void HandleMouseUp(const math::Vector& world_pos);
        virtual void HandleMousePosition(const math::Vector& world_pos);
        virtual void UpdateModifierState(bool ctrl, bool shift, bool alt);

        void MoveObjects(uint32_t entity_id, const math::Vector& delta);

    private:

        Editor* m_editor;
        uint32_t m_entity_id;
        math::Vector m_begin_translate;
    };
}
