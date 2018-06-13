
#pragma once

class Variant;
struct Attribute;

namespace editor
{
    void DrawProperty(const char* text, Variant& attribute);
    void DrawProperty(Attribute& attribute);
}
