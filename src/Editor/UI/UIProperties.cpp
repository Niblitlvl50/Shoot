
#include "UIProperties.h"
#include "ObjectAttribute.h"
#include "Component.h"
#include "UIContext.h"
#include "Resources.h"

#include "EntityLogicTypes.h"
#include "CollisionConfiguration.h"
#include "PickupTypes.h"

#include "ImGuiImpl/ImGuiImpl.h"

#include <cstdio>
#include <limits>

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

bool editor::DrawProperty(Attribute& attribute)
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
        std::vector<std::string> all_sprites = editor::GetAllSprites();
        const auto it = std::find(all_sprites.begin(), all_sprites.end(), (const char*)attribute.attribute);
        int selected_index = std::distance(all_sprites.begin(), it);

        const auto item_proxy = [](void* data, int idx, const char** out_text) -> bool
        {
            const std::vector<std::string>& strings = *(const std::vector<std::string>*)data;
            (*out_text) = strings[idx].c_str();
            return true;
        };


        //int selected_index = 0;
        const bool changed = ImGui::Combo(attribute_name, &selected_index, item_proxy, &all_sprites, all_sprites.size(), ImGuiComboFlags_HeightLarge);
        if(changed)
            attribute.attribute = all_sprites[selected_index].c_str();

        return changed;
/*
        const bool combo_opened = ImGui::BeginCombo(attribute_name, attribute.attribute, ImGuiComboFlags_HeightLarge);
        if(!combo_opened)
            return false;

        bool value_changed = false;
        const std::vector<std::string>& all_sprites = editor::GetAllSprites();
        for(size_t index = 0; index < all_sprites.size(); ++index)
        {
            const bool item_selected = ImGui::Selectable(all_sprites[index].c_str());
            if(item_selected)
            {
                value_changed = true;
                attribute.attribute = all_sprites[index].c_str();
            }
        }

        ImGui::EndCombo();
        return value_changed;
        */
    }
    else if(attribute.id == PATH_FILE_ATTRIBUTE)
    {
        const bool combo_opened = ImGui::BeginCombo(attribute_name, attribute.attribute, ImGuiComboFlags_HeightLarge);
        if(!combo_opened)
            return false;

        bool value_changed = false;
        const std::vector<std::string>& all_paths = editor::GetAllPaths();
        for(size_t index = 0; index < all_paths.size(); ++index)
        {
            const bool item_selected = ImGui::Selectable(all_paths[index].c_str());
            if(item_selected)
            {
                value_changed = true;
                attribute.attribute = all_paths[index].c_str();
            }
        }

        ImGui::EndCombo();
        return value_changed;
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
            if(DrawProperty(property))
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
            if(ImGui::Selectable(name.c_str()))
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

    const float item_width = ImGui::CalcItemWidth();
    const bool pushed = ImGui::Button(button_text.c_str(), ImVec2(item_width, 0));
    ImGui::SameLine();
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
