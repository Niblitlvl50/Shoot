
#include "ImGuiInterfaceDrawer.h"
#include "MainMenuOptions.h"
#include "UIContext.h"
#include "UIProperties.h"
#include "Resources.h"
#include "ObjectProxies/IObjectProxy.h"
#include "Util/Algorithm.h"

#include "Rendering/IRenderer.h"
#include "Rendering/Texture/ITexture.h"

#include "ImGuiImpl/ImGuiImpl.h"
#include "Component.h"
#include "EntitySystem/ObjectAttribute.h"

#include <algorithm>
#include <map>

using namespace editor;

namespace
{
    void DrawMainMenuBar(editor::UIContext& context)
    {
        ImGui::BeginMainMenuBar();
        if(ImGui::BeginMenu("Editor"))
        {
            if(ImGui::MenuItem("New Entity", "Ctrl + N"))
                context.editor_menu_callback(EditorMenuOptions::NEW);

            if(ImGui::MenuItem("Save", "Ctrl + S"))
                context.editor_menu_callback(EditorMenuOptions::SAVE);

            if(ImGui::MenuItem("Import Entity", "Ctrl + Shift + I"))
                context.editor_menu_callback(EditorMenuOptions::IMPORT_ENTITY);

            if(ImGui::MenuItem("Export Entity", "Ctrl + Shift + E"))
                context.editor_menu_callback(EditorMenuOptions::EXPORT_ENTITY);

            if(ImGui::MenuItem("Duplicate Entity", "Ctrl + D"))
                context.editor_menu_callback(EditorMenuOptions::DUPLICATE);

            if(ImGui::MenuItem("Re Export Entiites"))
                context.editor_menu_callback(EditorMenuOptions::REEXPORTENTITIES);

            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("Tools"))
        {
            if(ImGui::MenuItem("Translate", "1", context.active_tool_index == 0))
                context.tools_menu_callback(ToolsMenuOptions::TRANSLATE_TOOL);

            if(ImGui::MenuItem("Rotate", "2", context.active_tool_index == 1))
                context.tools_menu_callback(ToolsMenuOptions::ROTATE_TOOL);

            if(ImGui::MenuItem("Palette", "3", context.active_tool_index == 2))
                context.tools_menu_callback(ToolsMenuOptions::PALETTE_TOOL);

            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("Options"))
        {
            if(ImGui::Checkbox("Draw Object Names, N", &context.draw_object_names))
                context.draw_object_names_callback(context.draw_object_names);

            if(ImGui::Checkbox("Draw Snappers, S", &context.draw_snappers))
                context.draw_snappers_callback(context.draw_snappers);

            ImGui::Checkbox("Show Outline, O", &context.draw_outline);
            ImGui::Checkbox("Show Level Metadata, L", &context.draw_level_metadata);

            ImGui::Checkbox("Draw Grid, Alt + G", &context.draw_grid);
            ImGui::Checkbox("Snap to Grid, G", &context.snap_to_grid);
            ImGui::SameLine(0.0f, 50.0f);
            ImGui::InputFloat2("", &context.grid_size.x);

            ImGui::Checkbox("Draw all Objects, A", &context.draw_all_objects);
            if(ImGui::Checkbox("Draw Lights, Ctrl + L", &context.draw_lights))
                context.draw_lights_callback(context.draw_lights);

            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("Worlds"))
        {
            for(const std::string& world : context.all_worlds)
            {
                const bool is_selected = (world == context.selected_world);

                if(ImGui::MenuItem(world.c_str(), "", is_selected))
                    context.switch_world(world);
            }
            ImGui::EndMenu();
        }

        ImGui::SameLine(ImGui::GetWindowWidth() -250);
        ImGui::TextDisabled("mouse: %.2f %.2f", context.world_mouse_position.x, context.world_mouse_position.y);
        ImGui::SameLine(ImGui::GetWindowWidth() -100);
        ImGui::TextDisabled("fps: %.2f", context.fps);
        ImGui::EndMainMenuBar();
    }

    void DrawObjectOutline(editor::UIContext& context)
    {
        if(!context.draw_outline)
            return;

        std::map<std::string, std::vector<IObjectProxy*>> folder_to_proxies;
        std::vector<IObjectProxy*> orphan_proxies;

        for(const IObjectProxyPtr& proxy : *context.all_proxy_objects)
        {
            const std::string folder = proxy->GetFolder();
            if(folder.empty())
                orphan_proxies.push_back(proxy.get());
            else
                folder_to_proxies[folder].push_back(proxy.get());
        }

        constexpr int flags =
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoSavedSettings;

        const float window_height = ImGui::GetIO().DisplaySize.y;
        ImGui::SetNextWindowPos(ImVec2(30, 40));
        ImGui::SetNextWindowSizeConstraints(ImVec2(155, 50), ImVec2(155, window_height - 60));

        ImGui::Begin("Objects", nullptr, flags);

        const size_t n_objects = context.all_proxy_objects->size();
        ImGui::TextDisabled("%zu / 500 objects", n_objects);

        int id = 0;

        const auto add_selectables = [&id, &context](const std::vector<IObjectProxy*>& proxies)
        {
            for(IObjectProxy* proxy : proxies)
            {
                const bool in_selection =
                    std::find(context.selected_proxies.begin(), context.selected_proxies.end(), proxy) != context.selected_proxies.end();
                const bool pre_selection = (proxy == context.preselected_proxy_object);
                const bool selected = in_selection || pre_selection;
                
                ImGui::PushID(id++);

                if(ImGui::Selectable(proxy->Name().c_str(), selected, ImGuiSelectableFlags_AllowDoubleClick))
                {
                    const uint32_t entity_id = proxy->Id();
                    const bool is_double_click = ImGui::IsMouseDoubleClicked(0);
                    (is_double_click) ? context.teleport_to_object_callback(entity_id) : context.select_object_callback(entity_id);
                }

                if(ImGui::IsItemHovered())
                    context.preselect_object_callback(proxy);

                ImGui::PopID();
            }
        };

        for(const auto& pair : folder_to_proxies)
        {
            //ImGui::SetNextItemOpen(true);
            const bool open = ImGui::TreeNode(pair.first.c_str());
            if(open)
            {
                add_selectables(pair.second);
                ImGui::TreePop();
            }
        }

        add_selectables(orphan_proxies);

        ImGui::End();
    }

    void DrawSelectionView(editor::UIContext& context)
    {
        if(context.selected_proxies.empty())
            return;

        constexpr int flags =
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_AlwaysAutoResize;

        const float window_width = ImGui::GetIO().DisplaySize.x;
        const float window_height = ImGui::GetIO().DisplaySize.y;

        const float selection_width = 310;

        ImGui::SetNextWindowPos(ImVec2(window_width - selection_width - 30, 40));
        ImGui::SetNextWindowSizeConstraints(ImVec2(selection_width, 50), ImVec2(selection_width, window_height - 60));

        ImGui::Begin("Selection", nullptr, flags);

        // We could loop through all proxies and do a union of the components and present those.
        IObjectProxy* first_proxy = context.selected_proxies.front();
        std::vector<Component>& first_components = first_proxy->GetComponents();

        DrawAddComponent(context, first_components);
        const DrawComponentsResult result = DrawComponents(context, first_components);

        if(result.component_index != std::numeric_limits<uint32_t>::max())
        {
            const Component& src_component = first_components[result.component_index];
            const Attribute* src_attribute;
            FindAttribute(result.attribute_hash, src_component.properties, src_attribute);

            for(IObjectProxy* proxy : context.selected_proxies)
            {
                Component* target_component = FindComponentFromHash(result.component_hash, proxy->GetComponents());
                if(!target_component)
                    continue;

                Attribute* target_attribute;
                const bool found_it = FindAttribute(result.attribute_hash, target_component->properties, target_attribute);
                if(!found_it)
                    continue;

                target_attribute->value = src_attribute->value;

                proxy->ComponentChanged(*target_component, result.attribute_hash);
            }
        }

        ImGui::End();
    }

    void DrawLevelMetadata(editor::UIContext& context)
    {
        if(!context.draw_level_metadata)
            return;

        constexpr int flags =
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoSavedSettings;

        const float metadata_width = 310;
        const float window_width = ImGui::GetIO().DisplaySize.x;

        //ImGui::SetNextWindowSize(ImVec2(metadata_width, -1));
        ImGui::SetNextWindowPos(ImVec2(window_width / 2.0f - metadata_width / 2.0f, 40));

        ImGui::Begin("Level Metadata", nullptr, flags);

        ImGui::TextDisabled("Player");
        ImGui::InputFloat2("Spawn", &context.player_spawn_point.x);
        ImGui::Spacing();

        ImGui::TextDisabled("Game Camera");
        ImGui::InputFloat2("Position", &context.camera_position.x);
        ImGui::InputFloat2("Size", &context.camera_size.x);
        const float ratio = context.camera_size.x / context.camera_size.y;
        ImGui::TextDisabled("Ratio: %f", ratio);
        ImGui::Spacing();

        ImGui::TextDisabled("Colors");
        if(ImGui::ColorEdit3("Background Color", &context.background_color.red))
            context.background_color_callback(context.background_color);

        if(ImGui::ColorEdit3("Ambient Shade", &context.ambient_shade.red))
            context.ambient_shade_callback(context.ambient_shade);
        ImGui::Spacing();

        ImGui::TextDisabled("Background");
        const std::vector<std::string>& all_textures = editor::GetAllTextures();
        int out_index;
        const bool changed = editor::DrawStringPicker("Texture", context.background_texture, all_textures, out_index);
        if(changed)
        {
            context.background_texture = all_textures[out_index];
            context.background_texture_callback(context.background_texture);
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::TextDisabled("Triggers");
        static uint32_t selected_trigger_index = -1;
        editor::DrawListboxWidget("##triggers", context.triggers, selected_trigger_index);

        ImGui::Spacing();
        ImGui::TextDisabled("Conditions");
        static uint32_t selected_condition_index = -1;
        editor::DrawListboxWidget("##conditions", context.conditions, selected_condition_index);

        ImGui::End();
    }

    void DrawContextMenu(editor::UIContext& context)
    {
        if(context.show_context_menu)
        {
            ImGui::OpenPopup("context");
            context.show_context_menu = false;
        }

        if(ImGui::BeginPopup("context"))
        {
            int menu_index = -1;
            for(size_t index = 0; index < context.context_menu_items.size(); ++index)
            {
                if(ImGui::Selectable(context.context_menu_items.at(index).c_str()))
                    menu_index = index;
            }

            ImGui::EndPopup();

            if(menu_index != -1)
                context.context_menu_callback(menu_index);
        }
    }

    void DrawNotifications(editor::UIContext& context)
    {
        constexpr int notification_window_flags =
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoSavedSettings;

        const ImVec2 window_size = ImVec2(200.0f, 40.0f);
        const float window_position = (ImGui::GetIO().DisplaySize.x - window_size.x) / 2.0f;

        for(size_t index = 0; index < context.notifications.size(); ++index)
        {
            Notification& note = context.notifications[index];
            auto icon_it = context.ui_icons.find(note.icon);

            float window_alpha = 0.6f;
            ImColor tint = ImColor(1.0f, 1.0f, 1.0f, 1.0f);

            if(note.time_left < 500)
            {
                const float alpha_scale = float(note.time_left) / 500.0f;
                window_alpha *= alpha_scale;
                tint.Value.w *= alpha_scale;
            }

            char window_id[16];
            std::sprintf(window_id, "overlay: %zu", index);

            ImGui::SetNextWindowPos(ImVec2(window_position - 10, index * 60 + 30));
            ImGui::SetNextWindowSize(window_size);
            ImGui::SetNextWindowBgAlpha(window_alpha);

            ImGui::Begin(window_id, nullptr, notification_window_flags);

            if(icon_it != context.ui_icons.end())
            {
                void* texture_id = reinterpret_cast<void*>(icon_it->second.texture->Id());
                const ImageCoords& icon = QuadToImageCoords(icon_it->second.uv_upper_left, icon_it->second.uv_lower_right);
                ImGui::Image(texture_id, ImVec2(24.0f, 24.0f), icon.uv1, icon.uv2, tint);
                ImGui::SameLine();
            }

            ImGui::AlignTextToFramePadding();
            ImGui::Text("%s", note.text.c_str());
            ImGui::End();
        }
    }

    void DrawFileSelectionDialog(editor::UIContext& context)
    {
        constexpr const char* popup_name = "Select something...";
        if(context.show_modal_item_selection)
        {
            ImGui::OpenPopup(popup_name);
            context.show_modal_item_selection = false;
        }

        ImGui::SetNextWindowSize(ImVec2(500, -1));
        if(ImGui::BeginPopupModal(popup_name))
        {
            ImGui::Separator();

            int selected_index = -1;
            for(size_t index = 0; index < context.modal_items.size(); ++index)
            {
                const std::string& item = context.modal_items[index];
                if(ImGui::Selectable(item.c_str()))
                    selected_index = index;
            }

            ImGui::Separator();

            if(ImGui::Button("Cancel"))
                ImGui::CloseCurrentPopup();
            ImGui::EndPopup();

            if(selected_index != -1)
                context.modal_selection_callback(selected_index);
        }
    }

}

ImGuiInterfaceDrawer::ImGuiInterfaceDrawer(UIContext& context)
    : m_context(context)
{ }

void ImGuiInterfaceDrawer::Draw(mono::IRenderer& renderer) const
{
    DrawMainMenuBar(m_context);
    DrawObjectOutline(m_context);
    DrawSelectionView(m_context);
    DrawLevelMetadata(m_context);
    DrawContextMenu(m_context);
    DrawNotifications(m_context);
    DrawFileSelectionDialog(m_context);

//    ImGui::ShowDemoWindow();
    // Update UI stuff below

    const auto remove_notification_func = [&renderer](Notification& note) {
        note.time_left -= renderer.GetDeltaTimeMS();
        return note.time_left <= 0;
    };

    mono::remove_if(m_context.notifications, remove_notification_func);

    // Reset the shortcut
    m_context.open_add_component = false;
}

math::Quad ImGuiInterfaceDrawer::BoundingBox() const
{
    return math::InfQuad;
}
