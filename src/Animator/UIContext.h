
#pragma once

#include "Rendering/Sprite/SpriteData.h"
#include "Rendering/Texture/ITextureFactory.h"
#include "ImGuiImpl/ImGuiImageImpl.h"
#include "Math/Quad.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

namespace animator
{
    struct UIIcon
    {
        mono::IImGuiImagePtr imgui_image;
        math::Vector uv_upper_left;
        math::Vector uv_lower_right;
        math::Vector size;
        std::string category;
    };

    struct UIContext
    {
        mono::IImGuiImagePtr tools_image;
        math::Quad save_icon;
        math::Quad add_icon;
        math::Quad plus_icon;
        math::Quad delete_icon;

        std::unordered_map<std::string, UIIcon> ui_icons;

        bool offset_mode = false;
        bool animation_playing = false;
        float update_speed = 1.0f;

        int animation_id = 0;
        int selected_frame = 0;
        math::Vector frame_offset_pixels;

        std::string sprite_file;
        mono::SpriteData* sprite_data = nullptr;

        std::function<void ()> add_animation;
        std::function<void ()> delete_animation;
        std::function<void (int animation_id)> set_active_animation;

        // Active animation callbacks
        std::function<void (const char* new_name)> set_name;
        std::function<void (bool looping)> toggle_loop;
        std::function<void (int animation_frame_index, int frame)> animation_frame_updated;
        std::function<void (int new_frame_duration)> set_frame_duration;
        std::function<void ()> add_frame;
        std::function<void (int frame_id)> delete_frame;

        std::function<void (int frame)> set_active_frame;
        std::function<void (const math::Vector& frame_offset_pixels)> set_frame_offset;
        std::function<void ()> on_save;

        std::function<void ()> toggle_offset_mode;
        std::function<void ()> toggle_playing;
        std::function<void (float update_speed)> set_speed;

        std::function<void (const std::string& sprite_file)> open_sprite;
    };
}
