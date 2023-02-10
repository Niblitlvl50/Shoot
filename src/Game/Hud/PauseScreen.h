
#pragma once

#include "MonoFwd.h"
#include "UIElements.h"
#include "UI/UIItemProxy.h"

namespace game
{
    class PauseScreen : public game::UIElement
    {
    public:

        PauseScreen(
            mono::TransformSystem* transform_system,
            mono::InputSystem* input_system,
            mono::IEntityManager* entity_manager,
            mono::EventHandler* event_handler,
            class UISystem* ui_system);

        void ShowAt(const math::Vector& position);
        void Show() override;
        void Hide() override;
        void Update(const mono::UpdateContext& context) override;

        mono::TransformSystem* m_transform_system;
        mono::InputSystem* m_input_system;
        game::UISystem* m_ui_system;

        UITextElement* m_quit_text;
        UITextElement* m_close_text;
        UIItemProxy m_quit_proxy;
        UIItemProxy m_close_proxy;

        UITextureElement* m_gamepad_button_layout;
        UITextureElement* m_keyboard_key_layout;
    };
}
