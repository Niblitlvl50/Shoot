
#include "UIProperties.h"
#include "ObjectAttribute.h"
#include "ImGuiImpl/ImGuiImpl.h"

void editor::DrawProperty(const char* text, ObjectAttribute& attribute)
{
    switch(attribute.type)
    {
    case ObjectAttribute::Type::INT:
        ImGui::InputInt(text, &attribute.data.int_value);
        break;
    case ObjectAttribute::Type::FLOAT:
        ImGui::InputFloat(text, &attribute.data.float_value);
        break;
    case ObjectAttribute::Type::STRING:
        ImGui::InputText(text, attribute.data.string_value, 24);
        break;
    case ObjectAttribute::Type::POINT:
        ImGui::Value("X", attribute.data.point_value.x);
        ImGui::SameLine();
        ImGui::Value("Y", attribute.data.point_value.y);
        break;
    case ObjectAttribute::Type::NONE:
        break;
    }
}
