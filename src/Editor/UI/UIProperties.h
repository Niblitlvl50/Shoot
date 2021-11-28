
#pragma once

#include "Math/MathFwd.h"
#include "Rendering/RenderFwd.h"
#include "Rendering/Color.h"

#include <vector>
#include <string>
#include <cstdint>
#include <functional>

struct Attribute;
struct Component;

namespace editor
{
    bool DrawStringProperty(const char* name, std::string& value);
    bool DrawProperty(Attribute& attribute, const std::vector<Component>& all_components, struct UIContext& ui_context);
    void AddDynamicProperties(Component& component);

    struct DrawComponentsResult
    {
        uint32_t component_index;
        uint32_t component_hash;
        uint32_t attribute_hash;
    };
    DrawComponentsResult DrawComponents(struct UIContext& ui_context, std::vector<Component>& components);
    void DrawAddComponent(struct UIContext& ui_context, const std::vector<Component>& components);

    void DrawEntityProperty(uint32_t& properties);

    using FlagToStringFunc = const char*(*)(uint32_t flag);
    bool DrawBitfieldProperty(const char* name, uint32_t& value, const std::vector<uint32_t>& flags, FlagToStringFunc flag_to_string);

    bool DrawStringPicker(const char* name, const std::string& current_value, const std::vector<std::string>& all_strings, int& out_index);
    
    struct SpritePickerResult
    {
        bool changed;
        std::string new_value;
    };
    SpritePickerResult DrawSpritePicker(const char* name, const std::string& current_value, const UIContext& ui_context);

    bool DrawPolygonProperty(const char* name, std::vector<math::Vector>& polygon);
    bool DrawGradientProperty(const char* name, mono::Color::Gradient<4>& gradient);

    using EnablePickCallback = std::function<void (uint32_t* target)>;
    using SelectReferenceCallback = std::function<void (uint32_t reference)>;
    bool DrawEntityReferenceProperty(
        const char* name,
        const char* entity_name,
        uint32_t& entity_reference,
        const EnablePickCallback& pick_callback,
        const SelectReferenceCallback& select_callback);

    struct PaletteResult
    {
        int selected_index;
        bool apply;
        bool reset;
        std::string pasted_text;
    };

    PaletteResult DrawPaletteView(const std::vector<mono::Color::RGBA>& colors, int selected_index);

    bool DrawListboxWidget(const char* label, std::vector<std::string>& items, uint32_t& selected_index);
}
