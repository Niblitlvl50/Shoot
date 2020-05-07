
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
    void DrawEntityProperty(uint32_t& properties);
    bool DrawProperty(const char* text, Variant& attribute);
    bool DrawProperty(Attribute& attribute);
    void AddDynamicProperties(Component& component);
    int DrawComponents(struct UIContext& ui_context, std::vector<Component>& components);
}
