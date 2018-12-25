
#include "UIProperties.h"
#include "ObjectAttribute.h"
#include "DefinedAttributes.h"
#include "ImGuiImpl/ImGuiImpl.h"
#include "Util/Algorithm.h"

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
        ImGui::InputText(text, attribute.string_value, VariantStringMaxLength);
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

void editor::DrawProperty(Attribute& attribute)
{
    const char* attribute_name = world::AttributeNameFromHash(attribute.id);

    if(attribute.id == world::PICKUP_TYPE_ATTRIBUTE)
        ImGui::Combo(attribute_name, &attribute.attribute.int_value, world::PickupItems, mono::arraysize(world::PickupItems));
    else
        DrawProperty(attribute_name, attribute.attribute);
}

