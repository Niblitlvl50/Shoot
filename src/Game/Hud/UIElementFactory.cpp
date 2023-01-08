
#include "UIElementFactory.h"
#include "UIElements.h"

using namespace game;

UIElementFactory::UIElementFactory(
    mono::TransformSystem* transform_system, mono::IEntityManager* entity_manager, UISystem* ui_system)
    : m_transform_system(transform_system)
    , m_entity_manager(entity_manager)
    , m_ui_system(ui_system)
{

}

UISquareElement* UIElementFactory::CreateSquareElement(float width, float height, const mono::Color::RGBA& color)
{
    return new UISquareElement(width, height, color);
}

UITextElement* UIElementFactory::CreateTextElement(
    int font_id, const char* text, mono::FontCentering center_flags, const mono::Color::RGBA& color)
{
    return new UITextElement(font_id, text, center_flags, color);
}

UITextureElement* UIElementFactory::CreateTextureElement(const char* texture)
{
    return new UITextureElement(texture);
}
