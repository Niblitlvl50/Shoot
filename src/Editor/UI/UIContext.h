
#pragma once

#include "MainMenuOptions.h"
#include "Math/Quad.h"
#include "Rendering/Color.h"
#include "UINotification.h"

#include <functional>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

namespace editor
{
    constexpr const char* placeholder_texture = "res/textures/placeholder.png";
    constexpr const char* export_texture = "res/textures/icons/export.png";
    constexpr const char* import_texture = "res/textures/icons/import.png";
    constexpr const char* information_texture = "res/textures/icons/information.png";
    constexpr const char* save_texture = "res/textures/icons/save.png";
    constexpr const char* wrench_texture = "res/textures/icons/wrench.png";

    struct UIEntityItem
    {
        int texture_id = 0;
        math::Quad icon;
        std::string tooltip;
    };

    struct UIComponentItem
    {
        uint32_t hash;
        bool allow_multiple;
        std::string name;
    };

    struct UIFolder
    {
        std::string name;
        std::vector<uint32_t> proxy_ids;
    };

    struct UIIcon
    {
        int texture_id;
        math::Quad uv_coordinates;
    };

    struct UIContext
    {
        math::Vector world_mouse_position;

        // User tools
        int active_tool_index = 0;

        std::unordered_map<std::string, UIIcon> ui_icons;
        
        // Objects
        bool draw_outline = false;
        class IObjectProxy* selected_proxy_object = nullptr;
        class IObjectProxy* preselected_proxy_object = nullptr;
        std::vector<std::unique_ptr<IObjectProxy>>* all_proxy_objects = nullptr;
        std::vector<UIFolder> folders;

        // Options
        mono::Color::RGBA background_color;
        bool draw_object_names = false;
        bool draw_snappers = false;
        bool snap_to_grid = false;
        math::Vector grid_size;

        // Context menu
        bool show_context_menu = false;
        std::vector<std::string> context_menu_items;

        // Components
        std::vector<UIComponentItem> component_items;

        // Item selection
        bool show_modal_item_selection = false;
        std::vector<std::string> modal_items;

        // Notifications
        std::vector<Notification> notifications;

        // Callbacks
        std::function<void ()> delete_callback;
        
        std::function<void (uint32_t component_hash)> add_component;
        std::function<void (uint32_t index)> delete_component;
        std::function<void (IObjectProxy*)> select_object_callback;
        std::function<void (IObjectProxy*)> preselect_object_callback;
        std::function<void (IObjectProxy*)> teleport_to_object_callback;
        
        std::function<void (int)> context_menu_callback;
        std::function<void (int)> modal_selection_callback;
        std::function<void (EditorMenuOptions option)> editor_menu_callback;
        std::function<void (ToolsMenuOptions option)> tools_menu_callback;

        std::function<void (bool)> draw_object_names_callback;
        std::function<void (bool)> draw_snappers_callback;
        std::function<void (const mono::Color::RGBA&)> background_color_callback;
    };
}
