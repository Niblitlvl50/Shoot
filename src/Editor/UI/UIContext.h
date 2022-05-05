
#pragma once

#include "MainMenuOptions.h"
#include "Math/Quad.h"
#include "Rendering/Color.h"
#include "Rendering/Texture/ITextureFactory.h"
#include "UINotification.h"

#include "WorldFile.h"

#include <functional>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

namespace editor
{
    class IObjectProxy;

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

    using DecoratorFunc = bool (*)(const struct UIContext& ui_context, uint32_t component_index, Component& component);
    struct ComponentDecorator
    {
        DecoratorFunc header_decorator = nullptr;
        DecoratorFunc footer_decorator = nullptr;
    };

    struct UIContext
    {
        float fps;
        math::Vector world_mouse_position;
        std::unordered_map<std::string, UIIcon> ui_icons;
        mono::SystemContext* system_context;

        // User tools
        int active_tool_index = 0;

        // Level metadata
        bool draw_level_metadata = true;
        game::LevelMetadata level_metadata;
        
        // Objects
        uint32_t max_entities;
        bool draw_outline = false;
        std::vector<IObjectProxy*> selected_proxies;
        IObjectProxy* preselected_proxy_object = nullptr;
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
        std::unordered_map<uint32_t, ComponentDecorator> component_decorators;

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
        std::function<void ()> reset_zoom_callback;
        std::function<void ()> reset_position_callback;

        std::function<void (bool new_value)> draw_object_names_callback;
        std::function<void (bool new_value)> draw_snappers_callback;
        std::function<void (bool new_value)> draw_lights_callback;
        std::function<void (const mono::Color::RGBA& color)> background_color_callback;
        std::function<void (const mono::Color::RGBA& ambient_shade)> ambient_shade_callback;
        std::function<void (const math::Vector& size, const std::string& texture)> background_callback;

        std::function<const char* (uint32_t entity_id)> entity_name_callback;
    };
}
