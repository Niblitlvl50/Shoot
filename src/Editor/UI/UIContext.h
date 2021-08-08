
#pragma once

#include "MainMenuOptions.h"
#include "Math/Quad.h"
#include "Rendering/Color.h"
#include "Rendering/Texture/ITextureFactory.h"
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
        std::string category;
    };

    struct UIFolder
    {
        std::string name;
        std::vector<uint32_t> proxy_ids;
    };

    struct UIIcon
    {
        mono::ITexturePtr texture;
        math::Vector uv_upper_left;
        math::Vector uv_lower_right;
        math::Vector size;
        std::string category;
    };

    struct UIContext
    {
        float fps;
        math::Vector world_mouse_position;

        // User tools
        int active_tool_index = 0;

        std::unordered_map<std::string, UIIcon> ui_icons;

        // Level metadata
        bool draw_level_metadata = true;
        math::Vector camera_position;
        math::Vector camera_size;
        math::Vector player_spawn_point;
        mono::Color::RGBA background_color;
        mono::Color::RGBA ambient_shade;
        std::string background_texture;
        std::vector<std::string> triggers;
        std::vector<std::string> conditions;
        
        // Objects
        uint32_t max_entities;
        bool draw_outline = false;
        std::vector<class IObjectProxy*> selected_proxies;
        class IObjectProxy* preselected_proxy_object = nullptr;
        std::vector<std::unique_ptr<IObjectProxy>>* all_proxy_objects = nullptr;
        std::vector<UIFolder> folders;

        // Options
        bool draw_object_names = false;
        bool draw_snappers = false;
        bool draw_grid = false;
        bool snap_to_grid = false;
        math::Vector grid_size;
        bool draw_all_objects = false;
        bool draw_lights = true;

        // Worlds
        std::string selected_world;
        std::vector<std::string> all_worlds;

        // Context menu
        bool show_context_menu = false;
        std::vector<std::string> context_menu_items;

        // Components
        std::vector<UIComponentItem> component_items;
        bool open_add_component = false;

        // Item selection
        bool show_modal_item_selection = false;
        std::vector<std::string> modal_items;

        // Notifications
        std::vector<Notification> notifications;

        // Callbacks
        std::function<void ()> delete_callback;
        std::function<void (const std::string& new_world)> switch_world;
        std::function<void (uint32_t* target_data)> pick_callback;
        std::function<void (uint32_t entity_reference)> select_reference_callback;
        
        std::function<void (uint32_t component_hash)> add_component;
        std::function<void (uint32_t component_index)> delete_component;
        std::function<void (uint32_t entity_id)> select_object_callback;
        std::function<void (IObjectProxy* proxy)> preselect_object_callback;
        std::function<void (uint32_t entity_id)> teleport_to_object_callback;
        
        std::function<void (int)> context_menu_callback;
        std::function<void (int)> modal_selection_callback;
        std::function<void (EditorMenuOptions option)> editor_menu_callback;
        std::function<void (ToolsMenuOptions option)> tools_menu_callback;

        std::function<void (bool new_value)> draw_object_names_callback;
        std::function<void (bool new_value)> draw_snappers_callback;
        std::function<void (bool new_value)> draw_lights_callback;
        std::function<void (const mono::Color::RGBA& color)> background_color_callback;
        std::function<void (const mono::Color::RGBA& ambient_shade)> ambient_shade_callback;
        std::function<void (const std::string& background_texture)> background_texture_callback;

        std::function<const char* (uint32_t entity_id)> entity_name_callback;
    };
}
