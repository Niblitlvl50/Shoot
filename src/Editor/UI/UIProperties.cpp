
#include "UIProperties.h"
#include "EntitySystem/ObjectAttribute.h"
#include "Component.h"
#include "UIContext.h"
#include "Resources.h"

#include "EntityLogicTypes.h"
#include "CollisionConfiguration.h"
#include "PickupTypes.h"
#include "InteractionType.h"
#include "Math/EasingFunctions.h"
#include "Math/MathFunctions.h"
#include "TriggerTypes.h"
#include "AnimationModes.h"
#include "FontIds.h"
#include "Rendering/RenderSystem.h"
#include "Rendering/Sprite/ISpriteFactory.h"
#include "Rendering/Sprite/SpriteProperties.h"
#include "Rendering/Sprite/SpriteData.h"
#include "Rendering/Text/TextFlags.h"
#include "Rendering/Texture/ITexture.h"

#include "ImGuiImpl/ImGuiImpl.h"


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

bool editor::DrawStringProperty(const char* name, std::string& value)
{
    char text_buffer[VariantStringMaxLength] = { 0 };
    std::snprintf(text_buffer, VariantStringMaxLength, "%s", value.c_str());
    if(ImGui::InputText(name, text_buffer, VariantStringMaxLength))
    {
        value = text_buffer;
        return true;
    }

    return false;
}

bool DrawGenericProperty(const char* text, Variant& value)
{
    class DrawVariantsVisitor
    {
    public:

        DrawVariantsVisitor(const char* property_text)
            : m_property_text(property_text)
            , m_is_changed(false)
        { }

        void operator()(bool& value)
        {
            m_is_changed = ImGui::Checkbox(m_property_text, &value);
        }
        void operator()(int& value)
        {
            m_is_changed = ImGui::InputInt(m_property_text, &value);
        }
        void operator()(uint32_t& value)
        {
            ImGui::TextDisabled("Wait what, uint32_t has no UI!!!");
        }
        void operator()(float& value)
        {
            m_is_changed = ImGui::InputFloat(m_property_text, &value);
        }
        void operator()(math::Vector& value)
        {
            m_is_changed = ImGui::InputFloat2(m_property_text, &value.x);
        }
        void operator()(mono::Color::RGBA& value)
        {
            m_is_changed = ImGui::ColorEdit4(m_property_text, &value.red);
        }
        void operator()(std::string& value)
        {
            m_is_changed = editor::DrawStringProperty(m_property_text, value);
        }
        void operator()(std::vector<math::Vector>& value)
        {
            m_is_changed = editor::DrawPolygonProperty(m_property_text, value);
        }

        bool WasPropertyChanged() const
        {
            return m_is_changed;
        }

    private:

        const char* m_property_text;
        bool m_is_changed;
    };

    DrawVariantsVisitor visitor(text);
    std::visit(visitor, value);
    return visitor.WasPropertyChanged();
}

bool editor::DrawProperty(Attribute& attribute, const std::vector<Component>& all_components, UIContext& ui_context)
{
    const std::string text = PrettifyString(AttributeNameFromHash(attribute.id));
    const char* attribute_name = text.c_str();

    if(attribute.id == PICKUP_TYPE_ATTRIBUTE)
    {
        return ImGui::Combo(
            attribute_name, &std::get<int>(attribute.value), shared::pickup_items, std::size(shared::pickup_items));
    }
    else if(attribute.id == ENTITY_BEHAVIOUR_ATTRIBUTE)
    {
        return ImGui::Combo(
            attribute_name, &std::get<int>(attribute.value), shared::entity_logic_strings, std::size(shared::entity_logic_strings));
    }
    else if(attribute.id == BODY_TYPE_ATTRIBUTE)
    {
        return ImGui::Combo(
            attribute_name, &std::get<int>(attribute.value), body_types, std::size(body_types));
    }
    else if(attribute.id == FACTION_ATTRIBUTE)
    {
        const auto item_proxy = [](void* data, int idx, const char** out_text) -> bool
        {
            (*out_text) = shared::CollisionCategoryToString(shared::all_collision_categories[idx]);
            return true;
        };

        return ImGui::Combo(
            attribute_name, &std::get<int>(attribute.value), item_proxy, nullptr, std::size(shared::all_collision_categories));
    }
    else if(attribute.id == FACTION_PICKER_ATTRIBUTE)
    {
        return DrawBitfieldProperty(
            attribute_name, std::get<uint32_t>(attribute.value), shared::all_collision_categories, shared::CollisionCategoryToString);
    }
    else if(attribute.id == SPRITE_ATTRIBUTE)
    {
        const editor::SpritePickerResult result = DrawSpritePicker(attribute_name, std::get<std::string>(attribute.value), ui_context);
        if(result.changed)
            attribute.value = result.new_value;

        return result.changed;
    }
    else if(attribute.id == ANIMATION_ATTRIBUTE)
    {
        const Component* sprite_component = FindComponentFromHash(SPRITE_COMPONENT, all_components);

        std::string sprite_file;
        const bool success = FindAttribute(SPRITE_ATTRIBUTE, sprite_component->properties, sprite_file, FallbackMode::REQUIRE_ATTRIBUTE);
        if(!success)
            return false;

        char sprite_filename[1024] = { 0 };
        std::sprintf(sprite_filename, "res/sprites/%s", sprite_file.c_str());
        const mono::SpriteData* sprite_data = mono::GetSpriteFactory()->GetSpriteDataForFile(sprite_filename);
        if(!sprite_data)
            return false;

        std::vector<std::string> animation_names;

        for(const auto& animation : sprite_data->animations)
            animation_names.push_back(animation.name);

        // Make sure the animation index is within avalible ones
        attribute.value = std::min(std::get<int>(attribute.value), (int)sprite_data->animations.size() - 1);

        const int current_index = std::get<int>(attribute.value);
        int out_index = 0;
        const bool changed =
            DrawStringPicker(attribute_name, animation_names[current_index], animation_names, out_index);
        if(changed)
            attribute.value = out_index;
        return changed;
    }
    else if(attribute.id == SPRITE_PROPERTIES_ATTRIBUTE)
    {
        static const std::vector<uint32_t> all_sprite_properties = {
            mono::SpriteProperty::WIND_SWAY,
            mono::SpriteProperty::SHADOW,
            mono::SpriteProperty::FLIP_HORIZONTAL,
            mono::SpriteProperty::FLIP_VERTICAL,
            //mono::SpriteProperty::FLASH,
        };

        return DrawBitfieldProperty(
            attribute_name, std::get<uint32_t>(attribute.value), all_sprite_properties, mono::SpritePropertyToString);
    }
    else if(attribute.id == PATH_FILE_ATTRIBUTE)
    {
        const std::vector<std::string>& all_paths = editor::GetAllPaths();

        int out_index = 0;
        const bool changed = DrawStringPicker(attribute_name, std::get<std::string>(attribute.value), all_paths, out_index);
        if(changed)
            attribute.value = all_paths[out_index];
        
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
            attribute_name, &std::get<int>(attribute.value), item_proxy, nullptr, std::size(math::easing_function_strings));
    }
    else if(attribute.id == LOGIC_OP_ATTRIBUTE)
    {
        const auto item_proxy = [](void* data, int idx, const char** out_text) -> bool
        {
            (*out_text) = shared::AreaTriggerOpToString(shared::AreaTriggerOperation(idx));
            return true;
        };

        return ImGui::Combo(
            attribute_name, &std::get<int>(attribute.value), item_proxy, nullptr, std::size(shared::area_trigger_op_strings));
    }
    else if(attribute.id == ROTATION_ATTRIBUTE)
    {
        float degrees = math::ToDegrees(std::get<float>(attribute.value));
        const bool changed = ImGui::InputFloat(attribute_name, &degrees);
        if(changed)
            attribute.value = math::ToRadians(degrees);
        
        return changed;
    }
    else if(attribute.id == ANIMATION_MODE_ATTRIBUTE)
    {
        uint32_t int_value = std::get<int>(attribute.value);
        const bool changed = DrawBitfieldProperty(
            attribute_name, int_value, shared::all_animation_modes, shared::AnimationModeToString);
        if(changed)
            attribute.value = (int)int_value;

        return changed;
    }
    else if(attribute.id == FONT_ID_ATTRIBUTE)
    {
        const auto item_proxy = [](void* data, int idx, const char** out_text) -> bool
        {
            (*out_text) = shared::FontIdToString(shared::FontId(idx));
            return true;
        };

        return ImGui::Combo(
            attribute_name, &std::get<int>(attribute.value), item_proxy, nullptr, std::size(shared::font_id_strings));
    }
    else if(attribute.id == CENTER_FLAGS_ATTRIBUTE)
    {
        static const std::vector<uint32_t> all_centering_properties = {
            mono::FontCentering::VERTICAL,
            mono::FontCentering::HORIZONTAL,
        };

        return DrawBitfieldProperty(
            attribute_name, std::get<uint32_t>(attribute.value), all_centering_properties, mono::FontCenterToString);
    }
    else if(attribute.id == INTERACTION_TYPE_ATTRIBUTE)
    {
        const auto item_proxy = [](void* data, int idx, const char** out_text) -> bool
        {
            (*out_text) = shared::InteractionTypeToString(shared::InteractionType(idx));
            return true;
        };

        return ImGui::Combo(
            attribute_name, &std::get<int>(attribute.value), item_proxy, nullptr, std::size(shared::interaction_type_strings));
    }
    else
    {
        return DrawGenericProperty(attribute_name, attribute.value);
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
                std::string dummy_string;
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

editor::DrawComponentsResult editor::DrawComponents(UIContext& ui_context, std::vector<Component>& components)
{
    DrawComponentsResult result;
    result.component_index = -1;
    result.attribute_hash = -1;

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
    if(ImGui::Button("Add Component (Ctrl + A)", ImVec2(285, 0)) || ui_context.open_add_component)
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

        if(component.hash != TRANSFORM_COMPONENT)
        {
            ImGui::SameLine(245);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(70, 70, 70));
            if(ImGui::Button("Delete"))
                ui_context.delete_component(index);
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();
        }

        for(Attribute& property : component.properties)
        {
            if(DrawProperty(property, components, ui_context))
            {
                result.component_index = index;
                result.attribute_hash = property.id;
            }

            const char* tooltip = AttributeTooltipFromHash(property.id);
            if(tooltip && strlen(tooltip) > 0 && ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", tooltip);
        }

        ImGui::Spacing();
        ImGui::PopID();
    }
    
    constexpr uint32_t NOTHING_SELECTED = std::numeric_limits<uint32_t>::max();
    uint32_t selected_component_hash = NOTHING_SELECTED;

    if(ImGui::BeginPopup("select_component"))
    {
        std::string current_category;

        for(const UIComponentItem& component_item : ui_context.component_items)
        {
            if(current_category != component_item.category)
            {
                if(!current_category.empty())
                    ImGui::Separator();
                current_category = component_item.category;
            }

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

    return result;
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
    const char* name, const std::string& current_value, const std::vector<std::string>& all_strings, int& out_index)
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

editor::SpritePickerResult editor::DrawSpritePicker(const char* name, const std::string& current_value, const UIContext& ui_context)
{
    editor::SpritePickerResult result;
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
    const std::vector<std::string>& all_sprites = editor::GetAllSprites();

    struct SpriteUIIcon
    {
        std::string sprite_name;
        editor::UIIcon icon;
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

    ImGui::SetNextWindowSize(ImVec2(800, -1));
    if(ImGui::BeginPopup("sprite_picker_popup"))
    {
        ImGui::Columns(10, "sprite_picker_columns", false);

        for(size_t index = 0; index < sprite_icons.size(); ++index)
        {
            const SpriteUIIcon& sprite_icon = sprite_icons[index];
            ImGui::PushID(index);

            void* texture_id = reinterpret_cast<void*>(sprite_icon.icon.texture->Id());
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
                    ImDrawCornerFlags_All,
                    2.0f);
            }

            if(ImGui::IsItemHovered())
                ImGui::SetTooltip("%s|%s, width %.2f m, height %.2f m", sprite_icon.icon.category.c_str(), sprite_icon.sprite_name.c_str(), size.x, size.y);

            ImGui::NextColumn();
            ImGui::PopID();
        }

        ImGui::EndPopup();
    }

    return result;
}

bool editor::DrawPolygonProperty(const char* name, std::vector<math::Vector>& polygon)
{
    const ImGuiStyle& style = ImGui::GetStyle();
    const float item_width = ImGui::CalcItemWidth();

    const bool point_added = ImGui::Button("Add Point", ImVec2(item_width / 2.0f, 0));
    if(point_added)
    {
        const bool clockwise = math::IsPolygonClockwise(polygon);

        const math::Vector new_point = polygon.front() - polygon.back();
        const math::Vector new_point_tangent =
            clockwise ?
                math::Normalized(math::Vector(-new_point.y, new_point.x)) : 
                math::Normalized(math::Vector(new_point.y, -new_point.x));

        polygon.push_back(polygon.back() + (new_point / 2.0f) + (new_point_tangent * 0.25f));
    }

    ImGui::SameLine(0.0f, 1.0f);

    const bool point_removed = ImGui::Button("Remove Point", ImVec2(item_width / 2.0f, 0));
    if(point_removed && !polygon.empty())
        polygon.pop_back();

    ImGui::SameLine(0.0f, style.ItemInnerSpacing.x - 1.0f);
    ImGui::Text("%s", name);

    return point_added || point_removed;
}
