
#pragma once

#include "IUpdatable.h"

namespace editor
{
    struct UIContext;

    class ImGuiInterfaceDrawer : public mono::IUpdatable
    {
    public:
        ImGuiInterfaceDrawer(UIContext& context);
        virtual void doUpdate(const mono::UpdateContext& update_context);
    
    private:
        UIContext& m_context;
    };
}

