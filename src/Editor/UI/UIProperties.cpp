
#include "UIProperties.h"
#include "ObjectAttribute.h"
#include "ImGuiImpl/ImGuiImpl.h"

void editor::DrawProperty(const char* text, Variant& attribute)
{
    switch(attribute.type)
    {
    case Variant::Type::INT:
        ImGui::InputInt(text, &attribute.int_value);
        break;
    case Variant::Type::FLOAT:
        ImGui::InputFloat(text, &attribute.float_value);
        break;
    case Variant::Type::STRING:
        ImGui::InputText(text, attribute.string_value, 24);
        break;
    case Variant::Type::POINT:
        ImGui::Value("X", attribute.point_value.x);
        ImGui::SameLine();
        ImGui::Value("Y", attribute.point_value.y);
        break;
    case Variant::Type::NONE:
        break;
    }
}
