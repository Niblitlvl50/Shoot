
#pragma once

#include "MonoFwd.h"
#include "Rendering/RenderFwd.h"
#include "Rendering/Text/TextFlags.h"

namespace game
{
    class UIElementFactory
    {
    public:

        UIElementFactory(
            mono::TransformSystem* transform_system, mono::IEntityManager* entity_manager, class UISystem* ui_system);

        class UISquareElement* CreateSquareElement(
            float width, float height, const mono::Color::RGBA& color);
        
        class UITextElement* CreateTextElement(
            int font_id, const char* text, mono::FontCentering center_flags, const mono::Color::RGBA& color);
        
        class UITextureElement* CreateTextureElement(const char* texture);

    private:

        mono::TransformSystem* m_transform_system;
        mono::IEntityManager* m_entity_manager;
        UISystem* m_ui_system;
    };
}
