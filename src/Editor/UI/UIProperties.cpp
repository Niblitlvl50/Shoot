
#include "UIProperties.h"
#include "EntitySystem/ObjectAttribute.h"
#include "Component.h"
#include "UIContext.h"
#include "Resources.h"

#include "EntityLogicTypes.h"
#include "CollisionConfiguration.h"
#include "PickupTypes.h"
#include "Math/EasingFunctions.h"
#include "Math/MathFunctions.h"
#include "TriggerTypes.h"
#include "AnimationModes.h"
#include "FontIds.h"

#include "ImGuiImpl/ImGuiImpl.h"

#include "Rendering/Sprite/ISpriteFactory.h"
#include "Rendering/Sprite/SpriteData.h"

#include <cstdio>
#include <limits>
#include <algorithm>

std::string PrettifyString(const std::string& text)
{
    std::string out_string = text;
    std::replace(out_string.begin(), out_string.end(), '_', ' ');
    out_string[0] = std::toupper(out_string[0]);

    return out_string;
}

void editor::DrawName(std::string& name)
{
    char text_buffer[VariantStringMaxLength] = { 0 };
    std::snprintf(text_buffer, VariantStringMaxLength, "%s", name.c_str());
    if(ImGui::InputText("Name", text_buffer, VariantStringMaxLength))
        name = text_buffer;
}

void editor::DrawFolder(std::string& folder)
{
    char text_buffer[VariantStringMaxLength] = { 0 };
    std::snprintf(text_buffer, VariantStringMaxLength, "%s", folder.c_str());
    if(ImGui::InputText("Folder", text_buffer, VariantStringMaxLength))
        folder = text_buffer;
}

bool editor::DrawGenericProperty(const char* text, Variant& attribute)
{
    switch(attribute.type)
    {
    case Variant::Type::BOOL:
    {
        bool temp_value = attribute;
        const bool changed = ImGui::Checkbox(text, &temp_value);
        attribute = temp_value;
        return changed;
    }
    case Variant::Type::INT:
        return ImGui::InputInt(text, &attribute.int_value);
    case Variant::Type::UINT:
        //Imgui::InputU
        return false;
    case Variant::Type::FLOAT:
        return ImGui::InputFloat(text, &attribute.float_value);
    case Variant::Type::STRING:
    {
        char string_buffer[1024] = { 0 };
        std::snprintf(string_buffer, 1024, "%s", (const char*)attribute);
        const bool changed = ImGui::InputText(text, string_buffer, 1024);
        if(changed)
            attribute = string_buffer;
        return changed;
    }
    case Variant::Type::POINT:
        return ImGui::InputFloat2(text, &attribute.point_value.x);
    case Variant::Type::COLOR:
        return ImGui::ColorEdit4(text, &attribute.color_value.red);
    case Variant::Type::NONE:
        break;
    }

    return false;
}

bool editor::DrawProperty(Attribute& attribute, const std::vector<Component>& all_components, UIContext& ui_context)
{
    const std::string text = PrettifyString(AttributeNameFromHash(attribute.id));
    const char* attribute_name = text.c_str();

    if(attribute.id == PICKUP_TYPE_ATTRIBUTE)
    {
        return ImGui::Combo(
            attribute_name, &attribute.attribute.int_value, shared::pickup_items, std::size(shared::pickup_items));
    }
    else if(attribute.id == ENTITY_BEHAVIOUR_ATTRIBUTE)
    {
        return ImGui::Combo(
            attribute_name, &attribute.attribute.int_value, shared::entity_logic_strings, std::size(shared::entity_logic_strings));
    }
    else if(attribute.id == BODY_TYPE_ATTRIBUTE)
    {
        return ImGui::Combo(
            attribute_name, &attribute.attribute.int_value, body_types, std::size(body_types));
    }
    else if(attribute.id == FACTION_ATTRIBUTE)
    {
        const auto item_proxy = [](void* data, int idx, const char** out_text) -> bool
        {
            (*out_text) = shared::CollisionCategoryToString(shared::all_collision_categories[idx]);
            return true;
        };

        return ImGui::Combo(
            attribute_name, &attribute.attribute.int_value, item_proxy, nullptr, std::size(shared::all_collision_categories));
    }
    else if(attribute.id == FACTION_PICKER_ATTRIBUTE)
    {
        return DrawBitfieldProperty(
            attribute_name, attribute.attribute.uint_value, shared::all_collision_categories, shared::CollisionCategoryToString);
    }
    else if(attribute.id == SPRITE_ATTRIBUTE)
    {
        const editor::SpritePickerResult result = DrawSpritePicker(attribute_name, attribute.attribute, ui_context);
        if(result.changed)
            attribute.attribute = result.new_value.c_str();

        return result.changed;
    }
    else if(attribute.id == ANIMATION_ATTRIBUTE)
    {
        const Component* sprite_component = FindComponentFromHash(SPRITE_COMPONENT, all_components);

        const char* sprite_file = nullptr;

        const bool success = FindAttribute(SPRITE_ATTRIBUTE, sprite_component->properties, sprite_file, FallbackMode::REQUIRE_ATTRIBUTE);
        if(!success)
            return false;

        char sprite_filename[1024] = { 0 };
        std::sprintf(sprite_filename, "res/sprites/%s", sprite_file);
        const mono::SpriteData* sprite_data = mono::GetSpriteFactory()->GetSpriteDataForFile(sprite_filename);
        if(!sprite_data)
            return false;

        std::vector<std::string> animation_names;

        for(const auto& animation : sprite_data->animations)
            animation_names.push_back(animation.name);

        // Make sure the animation index is within avalible ones
        attribute.attribute = std::min((int)attribute.attribute, (int)sprite_data->animations.size() - 1);

        int out_index = 0;
        const bool changed =
            DrawStringPicker(attribute_name, animation_names[(int)attribute.attribute].c_str(), animation_names, out_index);
        if(changed)
            attribute.attribute = out_index;
        return changed;
    }
    else if(attribute.id == PATH_FILE_ATTRIBUTE)
    {
        const std::vector<std::string>& all_paths = editor::GetAllPaths();

        int out_index = 0;
        const bool changed = DrawStringPicker(attribute_name, attribute.attribute, all_paths, out_index);
        if(changed)
            attribute.attribute = all_paths[out_index].c_str();
        
        return changed;
    }
    else if(attribute.id == EASING_FUNC_ATTRIBUTE)
    {
        const auto item_proxy = [](void* data, int idx, const char** out_text) -> bool
        {
            (*out_text) = math::EasingFuncTypeToString(math::EasingFuncType(idx));
            return true;
        };

        return ImGui::Combo(
            attribute_name, &attribute.attribute.int_value, item_proxy, nullptr, std::size(math::easing_function_strings));
    }
    else if(attribute.id == LOGIC_OP_ATTRIBUTE)
    {
        const auto item_proxy = [](void* data, int idx, const char** out_text) -> bool
        {
            (*out_text) = shared::AreaTriggerOpToString(shared::AreaTriggerOperation(idx));
            return true;
        };

        return ImGui::Combo(
            attribute_name, &attribute.attribute.int_value, item_proxy, nullptr, std::size(shared::area_trigger_op_strings));
    }
    else if(attribute.id == ROTATION_ATTRIBUTE)
    {
        float degrees = math::ToDegrees(attribute.attribute);
        const bool changed = ImGui::InputFloat(text.c_str(), &degrees);
        if(changed)
            attribute.attribute = math::ToRadians(degrees);
        
        return changed;
    }
    else if(attribute.id == ANIMATION_MODE_ATTRIBUTE)
    {
        return DrawBitfieldProperty(
            attribute_name, attribute.attribute.uint_value, shared::all_animation_modes, shared::AnimationModeToString);
    }
    else if(attribute.id == FONT_ID_ATTRIBUTE)
    {
        const auto item_proxy = [](void* data, int idx, const char** out_text) -> bool
        {
            (*out_text) = shared::FontIdToString(shared::FontId(idx));
            return true;
        };

        return ImGui::Combo(
            attribute_name, &attribute.attribute.int_value, item_proxy, nullptr, std::size(shared::font_id_strings));
    }
    else
    {
        return DrawGenericProperty(attribute_name, attribute.attribute);
    }
}

void editor::AddDynamicProperties(Component& component)
{
    if(component.hash == BEHAVIOUR_COMPONENT)
    {
        int logic_type;
        const bool found_logic =
            FindAttribute(ENTITY_BEHAVIOUR_ATTRIBUTE, component.properties, logic_type, FallbackMode::REQUIRE_ATTRIBUTE);
        if(found_logic)
        {
            if(shared::EntityLogicType(logic_type) == shared::EntityLogicType::INVADER_PATH)
            {
                const char* dummy_string = nullptr;
                const bool has_path_file = FindAttribute(PATH_FILE_ATTRIBUTE, component.properties, dummy_string, FallbackMode::REQUIRE_ATTRIBUTE);
                if(!has_path_file)
                {
                    component.properties.push_back(
                        { PATH_FILE_ATTRIBUTE, DefaultAttributeFromHash(PATH_FILE_ATTRIBUTE) }
                    );
                }
            }
            else
            {
                StripUnknownProperties(component);
            }
        }
    }
}

int editor::DrawComponents(UIContext& ui_context, std::vector<Component>& components)
{
    int modified_index = -1;

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
    if(ImGui::Button("Add Component", ImVec2(285, 0)))
        ImGui::OpenPopup("select_component");
    ImGui::PopStyleVar();

    for(size_t index = 0; index < components.size(); ++index)
    {
        ImGui::Separator();
        ImGui::Spacing();

        Component& component = components[index];
        AddDynamicProperties(component);

        ImGui::PushID(index);

        const std::string name = PrettifyString(ComponentNameFromHash(component.hash));
        ImGui::TextDisabled("%s", name.c_str());
        ImGui::SameLine(245);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(70, 70, 70));
        if(ImGui::Button("Delete"))
            ui_context.delete_component(index);
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();

        for(Attribute& property : component.properties)
        {
            if(DrawProperty(property, components, ui_context))
                modified_index = index;
        }

        ImGui::Spacing();
        ImGui::PopID();
    }
    
    constexpr uint32_t NOTHING_SELECTED = std::numeric_limits<uint32_t>::max();
    uint32_t selected_component_hash = NOTHING_SELECTED;

    if(ImGui::BeginPopup("select_component"))
    {
        for(const UIComponentItem& component_item : ui_context.component_items)
        {
            const std::string name = PrettifyString(component_item.name);
            
            ImGuiSelectableFlags flags = 0;
            if(!component_item.allow_multiple)
            {
                const bool has_component = FindComponentFromHash(component_item.hash, components) != nullptr;
                if(has_component)
                    flags |= ImGuiSelectableFlags_Disabled;
            }

            if(ImGui::Selectable(name.c_str(), false, flags))
                selected_component_hash = component_item.hash;
        }
        ImGui::EndPopup();
    }

    if(selected_component_hash != NOTHING_SELECTED)
        ui_context.add_component(selected_component_hash);

    return modified_index;
}

bool editor::DrawBitfieldProperty(const char* name, uint32_t& value, const std::vector<uint32_t>& flags, FlagToStringFunc flag_to_string)
{
    std::string button_text = "[none]";

    if(value != 0)
    {
        button_text.clear();
        for(uint32_t prop : flags)
        {
            if(value & prop)
            {
                button_text += flag_to_string(prop);
                button_text += "|";
            }
        }

        button_text.pop_back();
    }

    const ImGuiStyle& style = ImGui::GetStyle();

    const float item_width = ImGui::CalcItemWidth();
    const bool pushed = ImGui::Button(button_text.c_str(), ImVec2(item_width, 0));
    ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);
    ImGui::Text("%s", name);
    if(pushed)
        ImGui::OpenPopup("entity_properties_select");

    const uint32_t initial_value = value;

    if(ImGui::BeginPopup("entity_properties_select"))
    {
        for(uint32_t prop : flags)
        {
            if(ImGui::Selectable(flag_to_string(prop), value & prop, ImGuiSelectableFlags_DontClosePopups))
                value ^= prop;
        }

        ImGui::EndPopup();
    }

    return initial_value != value;
}

bool editor::DrawStringPicker(
    const char* name, const char* current_value, const std::vector<std::string>& all_strings, int& out_index)
{
    const auto it = std::find(all_strings.begin(), all_strings.end(), current_value);
    out_index = std::distance(all_strings.begin(), it);

    const auto item_proxy = [](void* data, int idx, const char** out_text) -> bool
    {
        const std::vector<std::string>& strings = *(const std::vector<std::string>*)data;
        (*out_text) = strings[idx].c_str();
        return true;
    };

    return ImGui::Combo(
        name, &out_index, item_proxy, (void*)&all_strings, all_strings.size(), ImGuiComboFlags_HeightLarge);
}

editor::SpritePickerResult editor::DrawSpritePicker(const char* name, const char* current_value, const UIContext& ui_context)
{
    editor::SpritePickerResult result;
    result.changed = false;

    const float item_width = ImGui::CalcItemWidth();
    const bool pushed_button = ImGui::Button(current_value, ImVec2(item_width, 0));
    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
    ImGui::Text("%s", name);
    if(pushed_button)
        ImGui::OpenPopup("sprite_picker_popup");

    if(ImGui::IsPopupOpen("sprite_picker_popup"))
        ImGui::SetNextWindowSize(ImVec2(800, -1));
    
    if(ImGui::BeginPopup("sprite_picker_popup"))
    {
        const std::vector<std::string>& all_sprites = editor::GetAllSprites();
        ImGui::Columns(10, "sprite_picker_columns", false);

        for(const std::string& sprite : all_sprites)
        {
            ImGui::PushID(sprite.c_str());

            const auto it = ui_context.ui_icons.find(sprite);
            if(it != ui_context.ui_icons.end())
            {
                void* texture_id = reinterpret_cast<void*>(it->second.texture_id);
                const math::Quad& uv_coordinates = it->second.uv_coordinates;
                const math::Vector& size = it->second.size;

                constexpr float button_max_size = 64.0f;
                const float scale = (size.x > size.y) ? button_max_size / size.x : button_max_size / size.y;

                const ImageCoords& icon = QuadToImageCoords(uv_coordinates);
                const bool sprite_selected = ImGui::ImageButton(
                    texture_id, ImVec2(size.x * scale, size.y * scale), icon.uv1, icon.uv2, 2);
                if(sprite_selected)
                {
                    result.changed = true;
                    result.new_value = sprite;
                    ImGui::CloseCurrentPopup();
                }

                if(sprite == current_value)
                {
                    ImGui::GetWindowDrawList()->AddRect(
                        ImGui::GetItemRectMin(),
                        ImGui::GetItemRectMax(),
                        ImGui::GetColorU32(ImGuiCol_DragDropTarget),
                        3.0f,
                        ImDrawCornerFlags_All,
                        2.0f);
                }

                if(ImGui::IsItemHovered())
                    ImGui::SetTooltip("%s, width %.2f m, height %.2f m", sprite.c_str(), size.x, size.y);

                ImGui::NextColumn();
            }

            ImGui::PopID();
        }

        ImGui::EndPopup();
    }

    return result;
}
