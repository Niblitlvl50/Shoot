
#include "ImGuiInterfaceDrawer.h"
#include "MainMenuOptions.h"
#include "UIContext.h"
#include "ObjectProxies/IObjectProxy.h"
#include "Util/Algorithm.h"
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
            if(ImGui::MenuItem("New", "Ctrl + N"))
                context.editor_menu_callback(EditorMenuOptions::NEW);

            if(ImGui::MenuItem("Save", "Ctrl + S"))
                context.editor_menu_callback(EditorMenuOptions::SAVE);

            if(ImGui::MenuItem("Import Entity", "Ctrl + Shift + I"))
                context.editor_menu_callback(EditorMenuOptions::IMPORT_ENTITY);

            if(ImGui::MenuItem("Export Entity", "Ctrl + Shift + E"))
                context.editor_menu_callback(EditorMenuOptions::EXPORT_ENTITY);

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

            if(ImGui::Checkbox("Draw Object Names, N", &context.draw_object_names))
                context.draw_object_names_callback(context.draw_object_names);

            if(ImGui::Checkbox("Draw Snappers, S", &context.draw_snappers))
                context.draw_snappers_callback(context.draw_snappers);

            ImGui::Checkbox("Show Outline, O", &context.draw_outline);

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    void DrawObjectOutline(editor::UIContext& context)
    {
        if(!context.draw_outline)
            return;

        constexpr int flags =
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            //ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoSavedSettings;

        ImGui::SetNextWindowPos(ImVec2(30, 40));
        //ImGui::SetNextWindowSize(ImVec2(135, 600));

        ImGui::Begin("Objects", nullptr, flags);

        for(size_t index = 0; index < context.all_proxy_objects->size(); ++index)
        {
            const IObjectProxyPtr& proxy = context.all_proxy_objects->at(index);
            const bool selected = (proxy.get() == context.selected_proxy_object);
            
            ImGui::PushID(index);

            if(ImGui::Selectable(proxy->Name(), selected))
                context.select_object_callback(proxy.get());

            ImGui::PopID();
        }

        ImGui::End();
    }

    void DrawSelectionView(editor::UIContext& context)
    {
        if(!context.selected_proxy_object)
            return;

        constexpr int flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;
        ImGui::Begin("Selection", nullptr, ImVec2(250, 120), true, flags);

        context.selected_proxy_object->UpdateUIContext(context);

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

void ImGuiInterfaceDrawer::doUpdate(const mono::UpdateContext& update_context)
{
    ImGui::GetIO().DeltaTime = float(update_context.delta_ms) / 1000.0f;
    ImGui::NewFrame();

    DrawMainMenuBar(m_context);
    DrawObjectOutline(m_context);
    DrawSelectionView(m_context);
    DrawContextMenu(m_context);
    DrawNotifications(m_context);
    DrawFileSelectionDialog(m_context);

    //ImGui::ShowTestWindow();
    ImGui::Render();

    // Update UI stuff below

    const auto remove_notification_func = [update_context](Notification& note) {
        note.time_left -= update_context.delta_ms;
        return note.time_left <= 0;
    };

    mono::remove_if(m_context.notifications, remove_notification_func);
}
