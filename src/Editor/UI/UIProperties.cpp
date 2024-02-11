
#include "UIProperties.h"
#include "EntitySystem/ObjectAttribute.h"
#include "Entity/Component.h"
#include "UIContext.h"
#include "Resources.h"
#include "ObjectProxies/EditorEntityAttributes.h"

#include "Entity/AnimationModes.h"
#include "Entity/EntityLogicTypes.h"
#include "EntitySystem/Entity.h"
#include "CollisionConfiguration.h"
#include "Pickups/PickupTypes.h"
#include "InteractionSystem/InteractionType.h"
#include "Math/EasingFunctions.h"
#include "Math/MathFunctions.h"
#include "Sound/SoundSystem.h"
#include "TriggerSystem/TriggerTypes.h"
#include "FontIds.h"
#include "Rendering/RenderSystem.h"
#include "Rendering/Sprite/ISpriteFactory.h"
#include "Rendering/Sprite/SpriteProperties.h"
#include "Rendering/Sprite/SpriteData.h"
#include "Rendering/Text/TextFlags.h"
#include "Rendering/Texture/ITexture.h"
#include "Rendering/BlendMode.h"
#include "Particle/ParticleSystem.h"
#include "Paths/PathTypes.h"
#include "Physics/IBody.h"
#include "UI/UISystem.h"
#include "World/WorldBoundsTypes.h"
#include "World/WorldEntityTrackingSystem.h"

#include "ImGuiImpl/ImGuiImpl.h"
#include "imgui/imgui_internal.h"

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

    const size_t string_size = out_string.size();

    for(size_t index = 0; index < string_size; ++index)
    {
        const size_t next_index = index + 1;

        if(out_string[index] == ' ' && next_index < string_size)
            out_string[next_index] = std::toupper(out_string[next_index]);
    }

    return out_string;
}

bool editor::DrawStringProperty(const char* name, std::string& value, int imgui_flags)
{
    char text_buffer[1024] = { 0 };
    std::snprintf(text_buffer, std::size(text_buffer), "%s", value.c_str());
    if(ImGui::InputText(name, text_buffer, std::size(text_buffer), imgui_flags))
    {
        value = text_buffer;
        return true;
    }

    return false;
}

bool DrawGenericProperty(const char* text, Variant& value, const editor::UIContext& ui_context)
{
    struct DrawVariantsVisitor
    {
        const char* m_property_text;
        const editor::UIContext& m_ui_context;

        DrawVariantsVisitor(const char* property_text, const editor::UIContext& ui_context)
            : m_property_text(property_text)
            , m_ui_context(ui_context)
        { }
        bool operator()(bool& value)
        {
            return ImGui::Checkbox(m_property_text, &value);
        }
        bool operator()(int& value)
        {
            return ImGui::InputInt(m_property_text, &value);
        }
        bool operator()(uint32_t& value)
        {
            ImGui::TextDisabled("Wait what, uint32_t has no UI!!!");
            return false;
        }
        bool operator()(float& value)
        {
            return ImGui::InputFloat(m_property_text, &value, 0.0f, 0.0f, "%.5f");
        }
        bool operator()(math::Vector& value)
        {
            return ImGui::InputFloat2(m_property_text, &value.x);
        }
        bool operator()(mono::Color::RGBA& value)
        {
            return ImGui::ColorEdit4(m_property_text, &value.red);
        }
        bool operator()(std::string& value)
        {
            return editor::DrawStringProperty(m_property_text, value);
        }
        bool operator()(std::vector<math::Vector>& value)
        {
            return editor::DrawPolygonProperty(m_property_text, value);
        }
        bool operator()(math::Interval& value)
        {
            return ImGui::DragFloatRange2(m_property_text, &value.min, &value.max);
        }
        bool operator()(math::ValueSpread& value_spread)
        {
            return editor::DrawValueSpreadProperty(m_property_text, value_spread);
        }
        bool operator()(mono::Color::Gradient<4>& gradient)
        {
            return editor::DrawGradientProperty(m_property_text, gradient);
        }
        bool operator()(mono::Event& event)
        {
            return editor::DrawEventProperty(m_property_text, event, m_ui_context);
        }
    };

    DrawVariantsVisitor visitor(text, ui_context);
    return std::visit(visitor, value);
}

bool editor::DrawProperty(uint32_t component_hash, Attribute& attribute, const std::vector<Component>& all_components, UIContext& ui_context)
{
    const std::string text = PrettifyString(AttributeNameFromHash(attribute.id));
    const char* attribute_name = text.c_str();

    if(attribute.id == PICKUP_TYPE_ATTRIBUTE)
    {
        return ImGui::Combo(
            attribute_name, &std::get<int>(attribute.value), game::g_pickup_items, std::size(game::g_pickup_items));
    }
    else if(attribute.id == ENTITY_BEHAVIOUR_ATTRIBUTE)
    {
        return ImGui::Combo(
            attribute_name, &std::get<int>(attribute.value), game::g_entity_logic_strings, std::size(game::g_entity_logic_strings));
    }
    else if(attribute.id == BODY_TYPE_ATTRIBUTE)
    {
        return ImGui::Combo(
            attribute_name, &std::get<int>(attribute.value), mono::g_body_types, std::size(mono::g_body_types));
    }
    else if(attribute.id == FACTION_ATTRIBUTE)
    {
        const auto item_proxy = [](void* data, int idx, const char** out_text) -> bool
        {
            (*out_text) = game::CollisionCategoryToString(game::all_collision_categories[idx]);
            return true;
        };

        return ImGui::Combo(
            attribute_name, &std::get<int>(attribute.value), item_proxy, nullptr, std::size(game::all_collision_categories));
    }
    else if(attribute.id == FACTION_PICKER_ATTRIBUTE)
    {
        return DrawBitfieldProperty(
            attribute_name, std::get<uint32_t>(attribute.value), game::all_collision_categories, game::CollisionCategoryToString);
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
        const Component* sprite_component = component::FindComponentFromHash(SPRITE_COMPONENT, all_components);
        if(!sprite_component)
            return false;

        std::string sprite_file;
        const bool success = FindAttribute(SPRITE_ATTRIBUTE, sprite_component->properties, sprite_file, FallbackMode::REQUIRE_ATTRIBUTE);
        if(!success || sprite_file.empty())
            return false;

        char sprite_filename[1024] = { 0 };
        std::snprintf(sprite_filename, std::size(sprite_filename), "res/sprites/%s", sprite_file.c_str());
        const mono::SpriteData* sprite_data = mono::RenderSystem::GetSpriteFactory()->GetSpriteDataForFile(sprite_filename);
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
            (*out_text) = game::AreaTriggerOpToString(game::AreaTriggerOperation(idx));
            return true;
        };

        return ImGui::Combo(
            attribute_name, &std::get<int>(attribute.value), item_proxy, nullptr, std::size(game::area_trigger_op_strings));
    }
    else if(attribute.id == DESTROYED_TRIGGER_TYPE_ATTRIBUTE)
    {
        const auto item_proxy = [](void* data, int idx, const char** out_text) -> bool
        {
            (*out_text) = game::DestroyedTriggerTypeToString(game::DestroyedTriggerType(idx));
            return true;
        };

        return ImGui::Combo(
            attribute_name, &std::get<int>(attribute.value), item_proxy, nullptr, std::size(game::destroyed_trigger_type_strings));
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
            attribute_name, int_value, game::all_animation_modes, game::AnimationModeToString);
        if(changed)
            attribute.value = (int)int_value;

        return changed;
    }
    else if(attribute.id == FONT_ID_ATTRIBUTE)
    {
        const auto item_proxy = [](void* data, int idx, const char** out_text) -> bool
        {
            (*out_text) = game::FontIdToString(game::FontId(idx));
            return true;
        };

        return ImGui::Combo(
            attribute_name, &std::get<int>(attribute.value), item_proxy, nullptr, std::size(game::font_id_strings));
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
            (*out_text) = game::InteractionTypeToString(game::InteractionType(idx));
            return true;
        };

        return ImGui::Combo(
            attribute_name, &std::get<int>(attribute.value), item_proxy, nullptr, std::size(game::interaction_type_strings));
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
    else if(
        attribute.id == ENTITY_REFERENCE_ATTRIBUTE ||
        attribute.id == UI_LEFT_ITEM_ID_ATTRIBUTE ||
        attribute.id == UI_RIGHT_ITEM_ID_ATTRIBUTE ||
        attribute.id == UI_ABOVE_ITEM_ID_ATTRIBUTE ||
        attribute.id == UI_BELOW_ITEM_ID_ATTRIBUTE )
    {
        uint32_t& entity_id = std::get<uint32_t>(attribute.value);
        const char* entity_name = ui_context.entity_name_callback(entity_id);
        return DrawEntityReferenceProperty(
            attribute_name, entity_name, entity_id, component_hash, ui_context.pick_callback, ui_context.select_reference_callback);
    }
    else if(attribute.id == TEXTURE_ATTRIBUTE)
    {
        return DrawStringPicker(attribute_name, std::get<std::string>(attribute.value), editor::GetAllTextures());
    }
    else if(attribute.id == ENTITY_FILE_ATTRIBUTE)
    {
        return DrawStringPicker(attribute_name, std::get<std::string>(attribute.value), editor::GetAllEntities());
    }
    else if(attribute.id == BLEND_MODE_ATTRIBUTE)
    {
        const auto item_proxy = [](void* data, int idx, const char** out_text) -> bool
        {
            (*out_text) = mono::BlendModeToString(mono::BlendMode(idx));
            return true;
        };

        return ImGui::Combo(
            attribute_name, &std::get<int>(attribute.value), item_proxy, nullptr, std::size(mono::blend_mode_strings));
    }
    else if(attribute.id == TRANSFORM_SPACE_ATTRIBUTE)
    {
        const auto item_proxy = [](void* data, int idx, const char** out_text) -> bool
        {
            (*out_text) = mono::ParticleTransformSpaceToString(mono::ParticleTransformSpace(idx));
            return true;
        };

        return ImGui::Combo(
            attribute_name, &std::get<int>(attribute.value), item_proxy, nullptr, std::size(mono::particle_transform_space_strings));
    }
    else if(attribute.id == PARTICLE_DRAW_LAYER)
    {
        const auto item_proxy = [](void* data, int idx, const char** out_text) -> bool
        {
            (*out_text) = mono::ParticleDrawLayerToString(mono::ParticleDrawLayer(idx));
            return true;
        };

        return ImGui::Combo(
            attribute_name, &std::get<int>(attribute.value), item_proxy, nullptr, std::size(mono::particle_draw_layer_strings));
    }
    else if(attribute.id == POLYGON_DRAW_LAYER_ATTRIBUTE)
    {
        const auto item_proxy = [](void* data, int idx, const char** out_text) -> bool
        {
            (*out_text) = game::PolygonDrawLayerToString(game::PolygonDrawLayer(idx));
            return true;
        };

        return ImGui::Combo(
            attribute_name, &std::get<int>(attribute.value), item_proxy, nullptr, std::size(game::g_polygon_draw_layer_strings));
    }
    else if(attribute.id == EMITTER_TYPE_ATTRIBUTE)
    {
        const auto item_proxy = [](void* data, int idx, const char** out_text) -> bool
        {
            (*out_text) = mono::EmitterTypeToString(mono::EmitterType(idx));
            return true;
        };

        return ImGui::Combo(
            attribute_name, &std::get<int>(attribute.value), item_proxy, nullptr, std::size(mono::emitter_type_strings));
    }
    else if(attribute.id == EMITTER_MODE_ATTRIBUTE)
    {
        const auto item_proxy = [](void* data, int idx, const char** out_text) -> bool
        {
            (*out_text) = mono::EmitterModeToString(mono::EmitterMode(idx));
            return true;
        };

        return ImGui::Combo(
            attribute_name, &std::get<int>(attribute.value), item_proxy, nullptr, std::size(mono::emitter_mode_strings));
    }
    else if(attribute.id == EDITOR_PROPERTIES_ATTRIBUTE)
    {
        return DrawBitfieldProperty(
            attribute_name, std::get<uint32_t>(attribute.value), editor::all_entity_attributes, editor::EntityAttributeToString);
    }
    else if(
        attribute.id == WEAPON_PRIMARY_ATTRIBUTE ||
        attribute.id == WEAPON_SECONDARY_ATTRIBUTE ||
        attribute.id == WEAPON_TERTIARY_ATTRIBUTE )
    {
        return DrawStringPicker(attribute_name, std::get<std::string>(attribute.value), editor::GetAllWeapons());
    }
    else if(attribute.id == SOUND_ATTRIBUTE)
    {
        return DrawStringPicker(attribute_name, std::get<std::string>(attribute.value), editor::GetAllSounds());
    }
    else if(attribute.id == SOUND_PLAY_PARAMETERS)
    {
        return DrawBitfieldProperty(
            attribute_name, std::get<uint32_t>(attribute.value), game::all_sound_play_parameters, game::SoundInstancePlayParamterToString);
    }
    else if(attribute.id == UI_ITEM_STATE_ATTRIBUTE)
    {
        return ImGui::Combo(
            attribute_name, &std::get<int>(attribute.value), game::g_ui_item_state_strings, std::size(game::g_ui_item_state_strings));
    }
    else if(attribute.id == ENTITY_TYPE_ATTRIBUTE)
    {
        return ImGui::Combo(
            attribute_name, &std::get<int>(attribute.value), game::g_entity_type_strings, std::size(game::g_entity_type_strings));
    }
    else
    {
        return DrawGenericProperty(attribute_name, attribute.value, ui_context);
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
            if(game::EntityLogicType(logic_type) == game::EntityLogicType::INVADER_PATH)
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
                component::StripUnknownProperties(component);
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
        Component& component = components[index];

        const ComponentDecorator* decorator = nullptr;
        const auto decorator_it = ui_context.component_decorators.find(component.hash);
        if(decorator_it != ui_context.component_decorators.end())
            decorator = &decorator_it->second;

        ImGui::Separator();
        ImGui::Spacing();
        ImGui::PushID(index);

        AddDynamicProperties(component);

        const std::string name = PrettifyString(component::ComponentNameFromHash(component.hash));
        ImGui::TextDisabled("%s", name.c_str());

        if(component.hash != NAME_FOLDER_COMPONENT && component.hash != TRANSFORM_COMPONENT)
        {
            const ImVec2 size_avalible = ImGui::GetContentRegionAvail();
            ImGui::SameLine(size_avalible.x - 42.5f);

            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(70, 70, 70));
            if(ImGui::Button("Delete"))
                ui_context.delete_component(index);
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();
        }

        bool component_header_changed = false;
        bool component_changed = false;
        bool component_footer_changed = false;
        int component_attribute_hash = 0;

        if(decorator && decorator->header_decorator)
            component_header_changed = decorator->header_decorator(ui_context, index, component);

        for(Attribute& property : component.properties)
        {
            const bool property_changed = DrawProperty(component.hash, property, components, ui_context);
            if(property_changed)
            {
                component_changed = true;
                component_attribute_hash = property.id;
            }
            
            const char* tooltip = AttributeTooltipFromHash(property.id);
            if(tooltip && strlen(tooltip) > 0 && ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", tooltip);
        }

        if(decorator && decorator->footer_decorator)
            component_footer_changed = decorator->footer_decorator(ui_context, index, component);

        if(component_header_changed || component_changed || component_footer_changed)
        {
            result.component_index = index;
            result.component_hash = component.hash;
            result.attribute_hash = component_attribute_hash;
        }

        ImGui::Spacing();
        ImGui::PopID();
    }

    return result;
}

void editor::DrawAddComponent(UIContext& ui_context, const std::vector<Component>& components)
{
    const ImVec2 size_avalible = ImGui::GetContentRegionAvail();

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
    if(ImGui::Button("Add Component (Ctrl + A)", ImVec2(size_avalible.x, 0)) || ui_context.open_add_component)
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
                const bool has_component = component::FindComponentFromHash(component_item.hash, components) != nullptr;
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

bool editor::DrawStringPicker(const char* name, std::string& current_in_out_value, const std::vector<std::string>& all_strings)
{
    std::vector<std::string> local_all_strings = all_strings;
    local_all_strings.push_back("[none]");

    int out_index;
    const bool changed = editor::DrawStringPicker(name, current_in_out_value, local_all_strings, out_index);
    if(changed)
    {
        const std::string picked_value = local_all_strings[out_index];
        current_in_out_value = (picked_value == "[none]") ? "" : picked_value;
    }

    return changed;
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
    
                if(ImGui::IsWindowAppearing())
                    ImGui::SetScrollFromPosY(ImGui::GetItemRectMax().y);
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

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);

    const bool point_added = ImGui::Button("Add Point", ImVec2(item_width / 2.0f, 0));
    ImGui::SameLine(0.0f, 1.0f);
    const bool point_removed = ImGui::Button("Remove Point", ImVec2(item_width / 2.0f, 0));

    ImGui::PopStyleVar();

    ImGui::SameLine(0.0f, style.ItemInnerSpacing.x - 1.0f);
    ImGui::Text("%s", name);

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

    if(point_removed && !polygon.empty())
        polygon.pop_back();

    return point_added || point_removed;
}

bool editor::DrawValueSpreadProperty(const char* name, math::ValueSpread& value_spread)
{
    ImGui::PushID(name);
    ImGui::TextDisabled("%s", name);
    const bool value_changed = ImGui::InputFloat("Value", &value_spread.value);
    const bool spread_changed = ImGui::InputFloat2("Spread", &value_spread.spread.min);
    ImGui::PopID();
    return value_changed || spread_changed;
}

bool editor::DrawGradientProperty(const char* name, mono::Color::Gradient<4>& gradient)
{
    ImGui::Spacing();
    ImGui::TextDisabled("%s", "Gradient");

    const float item_width = ImGui::CalcItemWidth();
    const float item_width_4 = item_width / 4.0f;
    
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    ImGui::PushID("Colors");

    bool gradient_color_changed = false;

    for(int index = 0; index < 4; ++index)
    {
        ImGuiContext* g = ImGui::GetCurrentContext();

        mono::Color::RGBA& local_color = gradient.color[index];
        const ImVec4 imgui_color = ImVec4(local_color.red, local_color.green, local_color.blue, local_color.alpha);
        constexpr int button_flags = ImGuiColorEditFlags_AlphaPreview;

        ImGui::PushID(index);
        const bool pushed = ImGui::ColorButton("", imgui_color, button_flags, ImVec2(item_width_4, 0));
        if(pushed)
        {
            ImGui::GetCurrentContext()->ColorPickerRef = imgui_color;
            ImGui::OpenPopup("picker");
        }

        if (ImGui::BeginPopup("picker"))
        {
            ImVec4 imgui_color_picker = ImVec4(local_color.red, local_color.green, local_color.blue, local_color.alpha);
            const bool changed = ImGui::ColorPicker4("", &imgui_color_picker.x, 0, &g->ColorPickerRef.x);
            if(changed)
            {
                local_color.red = imgui_color_picker.x;
                local_color.green = imgui_color_picker.y;
                local_color.blue = imgui_color_picker.z;
                local_color.alpha = imgui_color_picker.w;
                gradient_color_changed = true;
            }

            ImGui::EndPopup();
        }

        ImGui::PopID();
        ImGui::SameLine();
    }
    ImGui::PopID();
    ImGui::PopStyleVar();

    const ImGuiStyle& style = ImGui::GetStyle();
    ImGui::SameLine(0, style.ItemInnerSpacing.x);
    ImGui::Text("Colors");

    const bool changed_0 = ImGui::DragFloat4("Fraction", gradient.t, 0.01f, 0.0f, 1.0f);
    ImGui::Spacing();

    return changed_0 || gradient_color_changed;
}

bool editor::DrawEventProperty(const char* name, mono::Event& event, const UIContext& ui_context)
{
    const ImGuiStyle& style = ImGui::GetStyle();
    const float item_width = ImGui::CalcItemWidth();
    const float item_spacing = style.ItemInnerSpacing.x;
    const float button_item_width = (item_width / 3.0f) - (item_spacing * 0.66f);

    ImGui::Spacing();
    ImGui::PushID(name);
    ImGui::BeginGroup();

    const char* icon_id =
        (event.direction == mono::EventDirection::Input) ? editor::event_in_texture : editor::event_out_texture;

    const auto icon_it = ui_context.ui_icons.find(icon_id);
    if(icon_it != ui_context.ui_icons.end())
    {
        ImTextureID texture_id = icon_it->second.imgui_image->TextureHandle();
        const ImageCoords& icon = QuadToImageCoords(icon_it->second.uv_upper_left, icon_it->second.uv_lower_right);
        ImGui::Image(texture_id, ImVec2(20.0f, 20.0f), icon.uv1, icon.uv2);
    }

    ImGui::SameLine();

    const float item_spacing_real = style.ItemSpacing.x;
    ImGui::SetNextItemWidth(item_width - 20.0f - item_spacing_real);

    const bool event_string_changed = DrawStringPicker(name, event.text, ui_context.level_metadata.triggers);

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);

    const auto draw_button = [&](const char* name, bool selected, bool same_line) {
        if(selected)
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.4f, 0.7f, 0.5f));

        const bool global_picked = ImGui::Button(name, ImVec2(button_item_width, 0.0f));
        if(selected)
            ImGui::PopStyleColor();

        if(same_line)
            ImGui::SameLine(0.0f, item_spacing);

        return global_picked;
    };

    const bool global_picked = draw_button("Global", event.type == mono::EventType::Global, true);
    if(global_picked)
        event.type = mono::EventType::Global;

    const bool local_picked = draw_button("Local", event.type == mono::EventType::Local, true);
    if(local_picked)
        event.type = mono::EventType::Local;

    const bool entity_picked = draw_button("Entity", event.type == mono::EventType::Entity, false);
    if(entity_picked)
        event.type = mono::EventType::Entity;

    ImGui::PopStyleVar();
    ImGui::EndGroup();
    ImGui::PopID();

    ImGui::Spacing();

    return event_string_changed || global_picked || local_picked || entity_picked;
}

bool editor::DrawEntityReferenceProperty(
    const char* name,
    const char* entity_name,
    uint32_t& entity_reference,
    uint32_t component_hash,
    const EnablePickCallback& pick_callback,
    const SelectReferenceCallback& select_callback)
{
    const ImGuiStyle& style = ImGui::GetStyle();
    const float item_width = ImGui::CalcItemWidth();

    const float item_spacing = style.ItemInnerSpacing.x - 1.0f;
    const float tiny_button_width = 19.0f;

    char label[32] = {};
    if(entity_reference == mono::INVALID_ID)
        std::snprintf(label, std::size(label), "None");
    else
        std::snprintf(label, std::size(label), "%s (%u)", entity_name, entity_reference);

    ImGui::PushID(&entity_reference);
    const bool enable_pick = ImGui::Button(label, ImVec2(item_width - (tiny_button_width * 2) - (item_spacing * 2), 0));
    if(enable_pick)
        pick_callback(component_hash, &entity_reference);

    ImGui::SameLine(0.0f, item_spacing);
    const bool clear_picked_entity = ImGui::Button("x", ImVec2(tiny_button_width, 0));
    if(clear_picked_entity)
        entity_reference = mono::INVALID_ID;

    ImGui::SameLine(0.0f, item_spacing);
    const bool select_picked_entity = ImGui::Button(">", ImVec2(tiny_button_width, 0));
    if(select_picked_entity)
        select_callback(entity_reference);

    ImGui::SameLine(0.0f, item_spacing);
    ImGui::Text("%s", name);

    ImGui::PopID();

    return enable_pick || clear_picked_entity;
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
                ImDrawFlags_RoundCornersAll,
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
        const bool is_shortcut_key = io.ConfigMacOSXBehaviors ? (io.KeyMods == ImGuiModFlags_Super) : (io.KeyMods == ImGuiModFlags_Ctrl);
        const bool is_paste = (is_shortcut_key && ImGui::IsKeyPressed(ImGuiKey(v_key), false));

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
                std::sort(items.begin(), items.end());
                changed = true;
            }
        }

        ImGui::EndPopup();
    }

    return changed;
}
