
#pragma once

#include <vector>
#include <string>
#include <cstdint>

class Variant;
struct Attribute;
struct Component;

namespace editor
{
    void DrawName(std::string& name);
    void DrawFolder(std::string& folder);
    bool DrawGenericProperty(const char* text, Variant& attribute);
    bool DrawProperty(Attribute& attribute, const std::vector<Attribute>& all_attributes);
    void AddDynamicProperties(Component& component);
    int DrawComponents(struct UIContext& ui_context, std::vector<Component>& components);

    void DrawEntityProperty(uint32_t& properties);

    using FlagToStringFunc = const char*(*)(uint32_t flag);
    bool DrawBitfieldProperty(const char* name, uint32_t& value, const std::vector<uint32_t>& flags, FlagToStringFunc flag_to_string);

    bool DrawStringPicker(const char* name, const char* current_value, const std::vector<std::string>& all_strings, int& out_index);
}
