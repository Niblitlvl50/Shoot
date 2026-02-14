
#include "InterfaceDrawer.h"
#include "UIContext.h"
#include "ImGuiImpl/ImGuiImpl.h"

#include "WriteSpriteFile.h"
#include "Rendering/Texture/ITexture.h"

#include <algorithm>

using namespace animator;

namespace
{
    constexpr int window_width = 300;

    void DrawOverlayToolbar(animator::UIContext& context)
    {
        ImGuiStyle& style = ImGui::GetStyle();
        const ImVec4& default_color = style.Colors[ImGuiCol_Button];
        const ImVec4& hovered_color = style.Colors[ImGuiCol_ButtonHovered];

        const ImGuiWindowFlags window_flags =
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoFocusOnAppearing |
            ImGuiWindowFlags_NoNav |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBackground;

        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));

        if (ImGui::Begin("button_overlay", nullptr, window_flags))
        {
            ImGui::PushStyleColor(ImGuiCol_Button, context.offset_mode ? hovered_color : default_color);
            if(ImGui::Button("Offset Mode"))
                context.toggle_offset_mode();
            ImGui::PopStyleColor();

            ImGui::SameLine();

            const char* play_pause_button = context.animation_playing ? "||" : "|>";
            ImGui::PushStyleColor(ImGuiCol_Button, context.animation_playing ? hovered_color : default_color);
            if(ImGui::Button(play_pause_button))
                context.toggle_playing();

            ImGui::PopStyleColor();

            ImGui::SameLine();
            if(ImGui::SliderFloat("Speed", &context.update_speed, 0.1f, 2.0f, "%.1f x"))
                context.set_speed(context.update_speed);

            ImGui::SameLine();

            const size_t slash_pos = context.sprite_file.find_last_of('/');
            const std::string sprite_file_name = (slash_pos != std::string::npos) ? context.sprite_file.substr(slash_pos + 1) : "[None]";

            const SpritePickerResult& result = animator::DrawSpritePicker("Sprite", sprite_file_name, context);
            if(result.changed)
            {
                const std::string sprite_file = "res/sprites/" + result.new_value;
                context.open_sprite(sprite_file);
            }
        }
        
        ImGui::End();
    }

    void DrawAnimationWindow(animator::UIContext& context)
    {
        ImGuiStyle& style = ImGui::GetStyle();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

        const ImVec2& window_size = ImGui::GetIO().DisplaySize;

        const int animation_window_height = window_size.y;
        const int window_x = window_size.x - window_width;

        //const ImageCoords& delete_icon = QuadToImageCoords(context.delete_icon);
        const ImageCoords& plus_icon = QuadToImageCoords(math::TopLeft(context.plus_icon), math::BottomRight(context.plus_icon));
        const ImageCoords& save_icon = QuadToImageCoords(math::TopLeft(context.save_icon), math::BottomRight(context.save_icon));

        //const ImVec4 bg_color(1.0f, 1.0f, 1.0f, 1.0f);
        const ImVec2 small_button_size(22, 22);
        const ImVec2 number_button_size(25, 0);
        const ImVec2 button_size(50, 21);

        const ImVec4& default_color = style.Colors[ImGuiCol_Button];
        const ImVec4& hovered_color = style.Colors[ImGuiCol_ButtonHovered];
        const ImVec4& window_bg_color = style.Colors[ImGuiCol_WindowBg];

        ImTextureID texture_id = context.tools_image->TextureHandle();

        ImGui::SetNextWindowPos(ImVec2(window_x, 0.0f));
        ImGui::SetNextWindowSize(ImVec2(window_width, animation_window_height));

        ImGui::Begin("Animation", nullptr, ImGuiWindowFlags_NoDecoration);
        ImGui::TextDisabled("ANIMATIONS");
        ImGui::Spacing();

        if(ImGui::Button("New", button_size))
            context.add_animation();

        ImGui::SameLine();
        if(ImGui::Button("Delete", button_size))
            context.delete_animation();

        ImGui::SameLine();
        ImGui::PushID(111);
        if(ImGui::ImageButton(texture_id, small_button_size, save_icon.uv1, save_icon.uv2, 0, window_bg_color))
            context.on_save();
        ImGui::PopID();

        ImGui::Spacing();
        ImGui::Spacing();

        ImGui::BeginChild("asdfasd", ImVec2(-1, 40), false, ImGuiWindowFlags_HorizontalScrollbar);

        for(size_t index = 0; index < context.sprite_data->animations.size(); ++index)
        {
            const bool is_selected_index = (index == (size_t)context.animation_id);

            char buffer[32] = { 0 };
            snprintf(buffer, 32, "%zu", index);

            const ImVec4& color = is_selected_index ? hovered_color : default_color;

            ImGui::PushStyleColor(ImGuiCol_Button, color);
            if(ImGui::Button(buffer, number_button_size))
                context.set_active_animation(index);

            if(is_selected_index)
                ImGui::SetScrollHereX();

            ImGui::PopStyleColor();
            ImGui::SameLine();
        }

        ImGui::EndChild();

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        mono::SpriteAnimation& active_animation = context.sprite_data->animations[context.animation_id];

        char buffer[100] = { 0 };
        snprintf(buffer, 100, "%s", active_animation.name.c_str());
        if(ImGui::InputText("##active_animation_button", buffer, 100))
            context.set_name(buffer);

        ImGui::SameLine();
        if(ImGui::Checkbox("Loop", &active_animation.looping))
            context.toggle_loop(active_animation.looping);

        if(ImGui::DragInt("Duration", &active_animation.frame_duration, 1.0f, 10, 1000, "%d ms"))
            context.set_frame_duration(active_animation.frame_duration);

        ImGui::Spacing();
        ImGui::Spacing();

        ImGui::AlignTextToFramePadding();
        ImGui::TextDisabled("FRAMES");
        ImGui::SameLine();

        ImGui::PushID(222);
        if(ImGui::ImageButton(texture_id, small_button_size, plus_icon.uv1, plus_icon.uv2, 0, window_bg_color))
            context.add_frame();
        ImGui::PopID();

        ImGui::Separator();
        ImGui::BeginChild(666);

        const float second_column_width = 120.0f;
        const float second_column_item_width = second_column_width - style.ItemSpacing.x * 2.0f;

        bool show_anim_notify_modal = false;

        ImGui::Columns(4, "animation_data", false);
        ImGui::SetColumnWidth(0, 30.0f);
        ImGui::SetColumnWidth(1, second_column_width);
        ImGui::SetColumnWidth(2, 60.0f);
        //ImGui::SetColumnWidth(3, 50.0f);

        for(size_t index = 0; index < active_animation.frames.size(); ++index)
        {
            mono::SpriteAnimationFrame animation_frame = active_animation.frames.at(index);
            const std::string string_index = std::to_string(index + 1);

            ImGui::PushID(index);
            ImGui::AlignTextToFramePadding();
            const bool selected = (index == (size_t)context.selected_frame);
            bool value = selected;
            const bool is_clicked = ImGui::Selectable(
                string_index.c_str(),
                &value,
                ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap);

            if(is_clicked && !selected)
                context.set_active_frame(index);

            ImGui::NextColumn();
            ImGui::SetNextItemWidth(second_column_item_width);
            if(ImGui::SliderInt("", &animation_frame.frame, 0, context.sprite_data->frames.size() -1))
                context.animation_frame_updated(index, animation_frame.frame);

            ImGui::NextColumn();

            const ImColor notify_color =
                animation_frame.notify.empty() ? ImColor(70, 70, 70) : ImColor(150, 150, 0);

            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)notify_color);
            if(ImGui::Button("Notify"))
            {
                context.set_active_frame(index);
                show_anim_notify_modal = true;
            }
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();

            ImGui::NextColumn();

            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(70, 70, 70));
            if(ImGui::Button("Delete"))
                context.delete_frame(index);
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();
            
            ImGui::NextColumn();
            ImGui::PopID();
            ImGui::Separator();
        }

        ImGui::EndChild();
        ImGui::End();

        ImGui::PopStyleVar(2);

        // NOTIFY MODAL DIALOG
        constexpr const char* notify_modal_name = "Notify";
        if(show_anim_notify_modal)
        {
            ImGui::OpenPopup(notify_modal_name);
        }

        if(ImGui::BeginPopupModal(notify_modal_name, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            mono::SpriteAnimationFrame& anim_frame = active_animation.frames[context.selected_frame];

            char text_buffer[1024] = { 0 };
            std::snprintf(text_buffer, std::size(text_buffer), "%s", anim_frame.notify.c_str());
            const bool changed = ImGui::InputText("##notify_input", text_buffer, std::size(text_buffer));
            if(changed)
            {
                anim_frame.notify = text_buffer;
            }

            ImGui::SameLine();

            if(ImGui::Button("Done", ImVec2(120, 0)))
            {
                ImGui::CloseCurrentPopup();
                //context.quit_callback();
            }

            ImGui::EndPopup();
        }

    }

    void DrawOffsetWindow(animator::UIContext& context)
    {
        if(!context.offset_mode)
            return;

        const int window_flags = 
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_AlwaysAutoResize;

        ImGui::Begin("offset_window", nullptr, window_flags);
        ImGui::TextDisabled("FRAME OFFSET");
        ImGui::Spacing();
        
        if(context.animation_playing)
        {
            ImGui::Text("Pause the animation to adjust the frame offset.");
        }
        else
        {
            const bool x_changed = ImGui::InputFloat("x", &context.frame_offset_pixels.x, 0.5f, 1.0f);
            const bool y_changed = ImGui::InputFloat("y", &context.frame_offset_pixels.y, 0.5f, 1.0f);
            if(x_changed || y_changed)
                context.set_frame_offset(context.frame_offset_pixels);
        }
        ImGui::End();
    }
}

InterfaceDrawer::InterfaceDrawer(UIContext& context)
    : m_context(context)
{ }

void InterfaceDrawer::Draw(mono::IRenderer& renderer) const
{
    (void)renderer;

    DrawOverlayToolbar(m_context);
    if(m_context.sprite_data)
    {
        DrawAnimationWindow(m_context);
        DrawOffsetWindow(m_context);
    }

    //ImGui::ShowDemoWindow();
}

math::Quad InterfaceDrawer::BoundingBox() const
{
    return math::InfQuad;
}


animator::SpritePickerResult animator::DrawSpritePicker(const char* name, const std::string& current_value, const UIContext& ui_context)
{
    animator::SpritePickerResult result;
    result.changed = false;

    const float item_width = ImGui::CalcItemWidth();
    const bool pushed_button = ImGui::Button(current_value.c_str(), ImVec2(item_width, 0));
    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
    ImGui::Text("%s", name);
    if(pushed_button)
        ImGui::OpenPopup("sprite_picker_popup");

    if(!ImGui::IsPopupOpen("sprite_picker_popup"))
        return result;

    // Prepare sprite image data
    const std::vector<std::string>& all_sprites = animator::GetAllSprites();

    struct SpriteUIIcon
    {
        std::string sprite_name;
        animator::UIIcon icon;
    };

    std::vector<SpriteUIIcon> sprite_icons;
    sprite_icons.reserve(all_sprites.size());

    for(const std::string& sprite_name : all_sprites)
    {
        const auto it = ui_context.ui_icons.find(sprite_name);
        if(it != ui_context.ui_icons.end())
            sprite_icons.push_back({sprite_name, it->second});
    }

    const auto sort_by_category = [](const SpriteUIIcon& first, const SpriteUIIcon& second)
    {
        if(first.icon.category == second.icon.category)
            return first.sprite_name < second.sprite_name;

        return first.icon.category < second.icon.category;
    };
    std::sort(sprite_icons.begin(), sprite_icons.end(), sort_by_category);

    std::string current_category;

    ImGui::SetNextWindowSize(ImVec2(800, -1));
    if(ImGui::BeginPopup("sprite_picker_popup"))
    {
        const bool begin_table = ImGui::BeginTable("sprite_picker_columns", 10);

        for(size_t index = 0; index < sprite_icons.size(); ++index)
        {
            ImGui::PushID(index);

            const SpriteUIIcon& sprite_icon = sprite_icons[index];

            if(current_category != sprite_icon.icon.category)
            {
                current_category = sprite_icon.icon.category;
                ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, IM_COL32(40, 40, 40, 127));
                ImGui::TableSetColumnIndex(0);
                ImGui::TextDisabled("%s", current_category.c_str());
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
            }

            const ImTextureID texture_id = sprite_icon.icon.imgui_image->TextureHandle();
            const math::Vector& size = sprite_icon.icon.size;

            constexpr float button_max_size = 64.0f;
            const float scale = (size.x > size.y) ? button_max_size / size.x : button_max_size / size.y;

            const ImageCoords& image_icon = QuadToImageCoords(sprite_icon.icon.uv_upper_left, sprite_icon.icon.uv_lower_right);
            const bool sprite_selected = ImGui::ImageButton(
                texture_id, ImVec2(size.x * scale, size.y * scale), image_icon.uv1, image_icon.uv2, 2);
            if(sprite_selected)
            {
                result.changed = true;
                result.new_value = sprite_icon.sprite_name;
                ImGui::CloseCurrentPopup();
            }

            if(sprite_icon.sprite_name == current_value)
            {
                ImGui::GetWindowDrawList()->AddRect(
                    ImGui::GetItemRectMin(),
                    ImGui::GetItemRectMax(),
                    ImGui::GetColorU32(ImGuiCol_DragDropTarget),
                    3.0f,
                    ImDrawFlags_RoundCornersAll,
                    2.0f);
            }

            if(ImGui::IsItemHovered())
                ImGui::SetTooltip("%s\nwidth %.2f m, height %.2f m", sprite_icon.sprite_name.c_str(), size.x, size.y);

            ImGui::TableNextColumn();
            ImGui::PopID();
        }

        if(begin_table)
            ImGui::EndTable();

        ImGui::EndPopup();
    }

    return result;
}
