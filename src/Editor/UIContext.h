
#pragma once

#include "MainMenuOptions.h"
#include "Math/Quad.h"
#include "UINotification.h"

#include <functional>
#include <vector>
#include <string>

namespace editor
{
    struct UIEntityItem
    {
        int texture_id = 0;
        math::Quad icon;
        std::string tooltip;
    };

    struct UIContext
    {
        // User tools
        int active_tool_index = 0;
        int tools_texture_id = 0;
        math::Quad default_icon;
        
        class IObjectProxy* proxy_object = nullptr;

        // Context menu
        bool showContextMenu = false;
        std::vector<std::string> contextMenuItems;

        // Entity panel + drag n drop
        std::string drag_context;
        std::vector<UIEntityItem> entity_items;

        std::vector<Notification> notifications;

        std::function<void ()> delete_callback;
        std::function<void (int)> context_menu_callback;
        std::function<void (EditorMenuOptions option)> editor_menu_callback;
        std::function<void (ToolsMenuOptions option)> tools_menu_callback;
        std::function<void (const std::string& id, const math::Vector& position)> drop_callback;
    };
}
