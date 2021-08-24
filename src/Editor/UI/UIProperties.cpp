
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
#include "Paths/PathTypes.h"

#include "ImGuiImpl/ImGuiImpl.h"

#include "System/Keycodes.h"
#include "System/System.h"

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
        if(!sprite_component)
            return false;

        std::string sprite_file;
        const bool success = FindAttribute(SPRITE_ATTRIBUTE, sprite_component->properties, sprite_file, FallbackMode::REQUIRE_ATTRIBUTE);
        if(!success || sprite_file.empty())
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
        const bool changed = DrawStringPicker(attribute_name, animation_names[current_index], animation_names, out_index);
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
    else if(attribute.id == DESTROYED_TRIGGER_TYPE_ATTRIBUTE)
    {
        const auto item_proxy = [](void* data, int idx, const char** out_text) -> bool
        {
            (*out_text) = shared::DestroyedTriggerTypeToString(shared::DestroyedTriggerType(idx));
            return true;
        };

        return ImGui::Combo(
            attribute_name, &std::get<int>(attribute.value), item_proxy, nullptr, std::size(shared::destroyed_trigger_type_strings));
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
    else if(attribute.id == PATH_TYPE_ATTRIBUTE)
    {
        const auto item_proxy = [](void* data, int idx, const char** out_text) -> bool
        {
            (*out_text) = mono::PathTypeToString(mono::PathType(idx));
            return true;
        };

        return ImGui::Combo(
            attribute_name, &std::get<int>(attribute.value), item_proxy, nullptr, std::size(mono::path_type_strings));
    }
    else if(attribute.id == ENTITY_REFERENCE_ATTRIBUTE)
    {
        uint32_t& entity_id = std::get<uint32_t>(attribute.value);
        const char* entity_name = ui_context.entity_name_callback(entity_id);
        return DrawEntityReferenceProperty(attribute_name, entity_name, entity_id, ui_context.pick_callback, ui_context.select_reference_callback);
    }
    else if(attribute.id == TEXTURE_ATTRIBUTE)
    {
        const std::vector<std::string>& all_textures = editor::GetAllTextures();

        int out_index = 0;
        const bool changed = DrawStringPicker(attribute_name, std::get<std::string>(attribute.value), all_textures, out_index);
        if(changed)
            attribute.value = all_textures[out_index];
        
        return changed;
    }
    else if(
        attribute.id == TRIGGER_NAME_ATTRIBUTE ||
        attribute.id == TRIGGER_NAME_EXIT_ATTRIBUTE ||
        attribute.id == ENABLE_TRIGGER_ATTRIBUTE ||
        attribute.id == DISABLE_TRIGGER_ATTRIBUTE ||
        attribute.id == TRIGGER_NAME_COMPLETED_ATTRIBUTE )
    {
        int out_index = 0;
        const bool changed = DrawStringPicker(attribute_name, std::get<std::string>(attribute.value), ui_context.triggers, out_index);
        if(changed)
            attribute.value = ui_context.triggers[out_index];
        
        return changed;
    }
    else if(attribute.id == CONDITION_ATTRIBUTE)
    {
        int out_index = 0;
        const bool changed = DrawStringPicker(attribute_name, std::get<std::string>(attribute.value), ui_context.conditions, out_index);
        if(changed)
            attribute.value = ui_context.conditions[out_index];
        
        return changed;
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
                uint32_t dummy_value;
                const bool has_path_file = FindAttribute(ENTITY_REFERENCE_ATTRIBUTE, component.properties, dummy_value, FallbackMode::REQUIRE_ATTRIBUTE);
                if(!has_path_file)
                {
                    component.properties.push_back(
                        { ENTITY_REFERENCE_ATTRIBUTE, DefaultAttributeFromHash(ENTITY_REFERENCE_ATTRIBUTE) }
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
    result.component_hash = -1;
    result.attribute_hash = -1;

    for(size_t index = 0; index < components.size(); ++index)
    {
        ImGui::Separator();
        ImGui::Spacing();

        Component& component = components[index];
        AddDynamicProperties(component);

        ImGui::PushID(index);

        const std::string name = PrettifyString(ComponentNameFromHash(component.hash));
        ImGui::TextDisabled("%s", name.c_str());

        if(component.hash != NAME_FOLDER_COMPONENT && component.hash != TRANSFORM_COMPONENT)
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
                result.component_hash = component.hash;
                result.attribute_hash = property.id;
            }

            const char* tooltip = AttributeTooltipFromHash(property.id);
            if(tooltip && strlen(tooltip) > 0 && ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", tooltip);
        }

        ImGui::Spacing();
        ImGui::PopID();
    }

    return result;
}

void editor::DrawAddComponent(UIContext& ui_context, const std::vector<Component>& components)
{
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
    if(ImGui::Button("Add Component (Ctrl + A)", ImVec2(285, 0)) || ui_context.open_add_component)
        ImGui::OpenPopup("select_component");

    ImGui::PopStyleVar();

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

bool editor::DrawEntityReferenceProperty(
    const char* name,
    const char* entity_name,
    uint32_t& entity_reference,
    const EnablePickCallback& pick_callback,
    const SelectReferenceCallback& select_callback)
{
    const ImGuiStyle& style = ImGui::GetStyle();
    const float item_width = ImGui::CalcItemWidth();

    const float item_spacing = style.ItemInnerSpacing.x - 1.0f;
    const float tiny_button_width = 19.0f;

    char label[32] = {};
    std::sprintf(label, "%s (%u)", entity_name, entity_reference);

    const bool enable_pick = ImGui::Button(label, ImVec2(item_width - tiny_button_width - item_spacing, 0));
    if(enable_pick)
        pick_callback(&entity_reference);

    ImGui::SameLine(0.0f, item_spacing);
    const bool select_picked_entity = ImGui::Button(">", ImVec2(tiny_button_width, 0));
    if(select_picked_entity)
        select_callback(entity_reference);

    ImGui::SameLine(0.0f, item_spacing);
    ImGui::Text("%s", name);

    return enable_pick;
}

editor::PaletteResult editor::DrawPaletteView(const std::vector<mono::Color::RGBA>& colors, int selected_index)
{
    PaletteResult result;
    result.selected_index = selected_index;
    result.apply = false;
    result.reset = false;

    constexpr int flags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_AlwaysAutoResize;

//    const float window_width = ImGui::GetIO().DisplaySize.x;
//    const float window_height = ImGui::GetIO().DisplaySize.y;
//    const float selection_width = 310;
//
//    ImGui::SetNextWindowPos(ImVec2(window_width - selection_width - 30, 40));
//    ImGui::SetNextWindowSizeConstraints(ImVec2(selection_width, 50), ImVec2(selection_width, window_height - 60));


    ImGui::Begin("Palette", nullptr, flags);

    for(uint32_t index = 0; index < colors.size(); ++index)
    {
        const mono::Color::RGBA& color = colors[index];
        const ImVec4 imgui_color = ImVec4(color.red, color.green, color.blue, color.alpha);

        ImGui::PushID(index);
        ImGui::PushStyleColor(ImGuiCol_Button, imgui_color);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, imgui_color);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, imgui_color);
        if(ImGui::Button("", ImVec2(80.0f, 80.0f)))
            result.selected_index = index;

        ImGui::PopStyleColor(3);
        ImGui::PopID();

        if(index == uint32_t(selected_index))
        {
            ImGui::GetWindowDrawList()->AddRect(
                ImGui::GetItemRectMin(),
                ImGui::GetItemRectMax(),
                ImGui::GetColorU32(ImGuiCol_DragDropTarget),
                3.0f,
                ImDrawCornerFlags_All,
                2.0f);
        }
    }

    result.apply = ImGui::Button("Apply", ImVec2(80, 0));
    result.reset = ImGui::Button("Reset", ImVec2(80, 0));

    const bool in_focus = ImGui::IsWindowFocused();
    if(in_focus)
    {
        const int v_key = System::KeycodeToNative(Keycode::V);

        const ImGuiIO& io = ImGui::GetIO();
        const bool is_shortcut_key = io.ConfigMacOSXBehaviors ? (io.KeyMods == ImGuiKeyModFlags_Super) : (io.KeyMods == ImGuiKeyModFlags_Ctrl);
        const bool is_paste = (is_shortcut_key && ImGui::IsKeyPressed(v_key, false));

        if(is_paste)
        {
            const char* clipboard_text = ImGui::GetClipboardText();
            if(clipboard_text)
                result.pasted_text = clipboard_text;
       }
    }

    ImGui::End();

    return result;
}

bool editor::DrawListboxWidget(const char* label, std::vector<std::string>& items, uint32_t& selected_index)
{
    bool changed = false;

    const bool triggers_box_open = ImGui::BeginListBox(label);
    if(triggers_box_open)
    {
        for(uint32_t index = 0; index < items.size(); ++index)
        {
            const bool is_selected = (index == selected_index);
            const bool new_index_selected = ImGui::Selectable(items[index].c_str(), is_selected);
            if(new_index_selected)
                selected_index = index;
        }
        ImGui::EndListBox();
    }

    const std::string add_button_label = "Add" + std::string("##") + label + "add";
    const std::string remove_button_label = "Remove" + std::string("##") + label + "remove";

    const bool add_trigger = ImGui::Button(add_button_label.c_str());
    ImGui::SameLine();
    const bool remove_trigger = ImGui::Button(remove_button_label.c_str());
    if(remove_trigger)
    {
        if(items.size() > selected_index)
        {
            items.erase(items.begin() + selected_index);
            changed = true;
        }
    }

    if(add_trigger)
        ImGui::OpenPopup(add_button_label.c_str());

    const bool add_trigger_open = ImGui::BeginPopupModal(add_button_label.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    if(add_trigger_open)
    {
        char text_buffer[1024] = { };
        ImGui::InputText("Name", text_buffer, std::size(text_buffer), ImGuiInputTextFlags_CharsNoBlank);
        const bool hit_enter = ImGui::IsItemDeactivatedAfterEdit();

        ImGui::Spacing();
        ImGui::Separator();

        const bool hit_ok = ImGui::Button("OK", ImVec2(120, 0)) || hit_enter;
        ImGui::SameLine();
        const bool hit_cancel = ImGui::Button("Cancel", ImVec2(120, 0));

        if(hit_ok || hit_cancel)
            ImGui::CloseCurrentPopup();

        if(hit_ok)
        {
            std::string new_trigger_name = text_buffer;
            if(!new_trigger_name.empty())
            {
                items.push_back(new_trigger_name);
                changed = true;
            }
        }

        ImGui::EndPopup();
    }

    return changed;
}
