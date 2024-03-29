
#pragma once

#include "MonoFwd.h"
#include "UIElements.h"
#include "UI/UIItemProxy.h"

namespace game
{
    class ShopScreen : public game::UIElement
    {
    public:

        ShopScreen(
            mono::TransformSystem* transform_system,
            mono::IEntityManager* entity_manager,
            mono::EventHandler* event_handler,
            class UISystem* ui_system,
            class ShopSystem* shop_system);

        void ShowAt(const math::Vector& position);
        void Show() override;
        void Hide() override;

        game::UISystem* m_ui_system;
        game::ShopSystem* m_shop_system;
        UITextElement* m_close_text;
        UIItemProxy m_close_proxy;
    };
}
