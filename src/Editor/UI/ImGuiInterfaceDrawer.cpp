
#include "ImGuiInterfaceDrawer.h"
#include "MainMenuOptions.h"
#include "UIContext.h"
#include "ObjectProxies/IObjectProxy.h"
#include "Algorithm.h"
#include "ImGuiImpl/ImGuiImpl.h"

#include <algorithm>

using namespace editor;

namespace
{
    void DrawMainMenuBar(editor::UIContext& context)
    {
        ImGui::BeginMainMenuBar();
        if(ImGui::BeginMenu("Editor"))
        {
            if(ImGui::MenuItem("Save", "Ctrl + S"))
                context.editor_menu_callback(EditorMenuOptions::SAVE);

            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("Tools"))
        {
            if(ImGui::MenuItem("Translate", "1", context.active_tool_index == 0))
                context.tools_menu_callback(ToolsMenuOptions::TRANSLATE_TOOL);

            if(ImGui::MenuItem("Rotate", "2", context.active_tool_index == 1))
                context.tools_menu_callback(ToolsMenuOptions::ROTATE_TOOL);

            if(ImGui::MenuItem("Polygon", "3", context.active_tool_index == 2))
                context.tools_menu_callback(ToolsMenuOptions::POLYGON_TOOL);

            if(ImGui::MenuItem("Polygon Brush", "4", context.active_tool_index == 3))
                context.tools_menu_callback(ToolsMenuOptions::POLYGON_BRUSH_TOOL);

            if(ImGui::MenuItem("Path drawer", "5", context.active_tool_index == 4))
                context.tools_menu_callback(ToolsMenuOptions::PATH_TOOL);

            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("Options"))
        {
            if(ImGui::ColorEdit3("Background Color", &context.background_color.red))
                context.background_color_callback(context.background_color);

            if(ImGui::Checkbox("Draw Object Names", &context.draw_object_names))
                context.draw_object_names_callback(context.draw_object_names);

            if(ImGui::Checkbox("Draw Snappers", &context.draw_snappers))
                context.draw_snappers_callback(context.draw_snappers);

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    void DrawObjectView(editor::UIContext& context)
    {
        constexpr int flags =
            //ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoSavedSettings;

        ImGui::SetNextWindowPos(ImVec2(30, 40));
        //ImGui::SetNextWindowSize(ImVec2(135, 600));

        ImGui::Begin("Objects", nullptr, flags);

        ImGui::RadioButton("Entites", &context.active_panel_index, 0); ImGui::SameLine();
        ImGui::RadioButton("Prefabs", &context.active_panel_index, 1);

        ImGui::Spacing();

        ImGui::Columns(2, nullptr, false);

        const std::vector<UIEntityItem>& objects_to_draw
            = (context.active_panel_index == 0) ? context.entity_items : context.prefab_items;

        for(size_t index = 0; index < objects_to_draw.size(); ++index)
        {
            const UIEntityItem& item = objects_to_draw[index];

            void* texture_id = reinterpret_cast<void*>(item.texture_id);
            const ImageCoords& icon = QuadToImageCoords(item.icon);

            ImGui::PushID(index);
            //ImGui::ImageButton(texture_id, ImVec2(48.0f, 48.0f), icon.uv1, icon.uv2, 0);
            ImGui::ImageButton(texture_id, ImVec2(64.0f, 64.0f), icon.uv1, icon.uv2, 0);

            if(ImGui::IsItemActive() && ImGui::IsMouseDragging())
                context.drag_context = item.tooltip;
            else if(ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", item.tooltip.c_str());

            ImGui::NextColumn();
            ImGui::PopID();
        }

        if(!context.drag_context.empty())
        {
            if(ImGui::IsMouseDragging())
            {
                ImGui::SetTooltip("%s", context.drag_context.c_str());
            }
            else if(ImGui::IsMouseReleased(0))
            {
                const ImVec2& mouse_pos = ImGui::GetMousePos();
                context.drop_callback(context.drag_context, math::Vector(mouse_pos.x, mouse_pos.y));
                context.drag_context.clear();
            }
        }

        ImGui::End();
    }

    void DrawSelectionView(editor::UIContext& context)
    {
        if(!context.proxy_object)
            return;

        constexpr int flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;
        ImGui::Begin("Selection", nullptr, ImVec2(250, 120), true, flags);

        context.proxy_object->UpdateUIContext(context);

        if(ImGui::Button("Delete"))
            context.delete_callback();

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

        const ImVec2 window_size = ImVec2(160.0f, 50.0f);
        const float window_position = ImGui::GetIO().DisplaySize.x - window_size.x;
        void* texture_id = reinterpret_cast<void*>(context.tools_texture_id);

        for(size_t index = 0; index < context.notifications.size(); ++index)
        {
            Notification& note = context.notifications[index];

            float window_alpha = 0.6f;
            ImColor tint = ImColor(1.0f, 1.0f, 1.0f, 1.0f);

            if(note.time_left < 500)
            {
                const float alpha_scale = float(note.time_left) / 500.0f;
                window_alpha *= alpha_scale;
                tint.Value.w *= alpha_scale;
            }

            const ImageCoords& icon = QuadToImageCoords(note.icon);

            char window_id[16];
            std::sprintf(window_id, "overlay: %zu", index);

            ImGui::SetNextWindowPos(ImVec2(window_position - 10, index * 60 + 30));

            ImGui::Begin(window_id, nullptr, window_size, window_alpha, notification_window_flags);
            ImGui::Image(texture_id, ImVec2(32.0f, 32.0f), icon.uv1, icon.uv2, tint);
            ImGui::SameLine();
            ImGui::TextColored(tint, "%s", note.text.c_str());
            ImGui::End();
        }
    }
}

ImGuiInterfaceDrawer::ImGuiInterfaceDrawer(UIContext& context)
    : m_context(context)
{ }

void ImGuiInterfaceDrawer::doUpdate(unsigned int delta)
{
    ImGui::GetIO().DeltaTime = float(delta) / 1000.0f;
    ImGui::NewFrame();

    DrawMainMenuBar(m_context);
    DrawObjectView(m_context);
    DrawSelectionView(m_context);    
    DrawContextMenu(m_context);
    DrawNotifications(m_context);

    //ImGui::ShowTestWindow();
    ImGui::Render();

    // Update UI stuff below

    const auto remove_notification_func = [delta](Notification& note) {
        note.time_left -= delta;
        return note.time_left <= 0;
    };

    mono::remove_if(m_context.notifications, remove_notification_func);
}
