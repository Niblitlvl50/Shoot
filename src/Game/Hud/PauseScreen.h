
#pragma once

#include "MonoFwd.h"
#include "UIElements.h"
#include "UI/UIItemProxy.h"

#include <string>

namespace game
{
    class PauseScreen : public game::UIElement
    {
    public:

        PauseScreen(
            const std::string& aborted_hash,
            mono::TransformSystem* transform_system,
            mono::InputSystem* input_system,
            mono::IEntityManager* entity_manager,
            mono::EventHandler* event_handler,
            class UISystem* ui_system,
            float width = 10.0f,
            float height = 5.5f);

        void ShowAt(const math::Vector& position);
        void Show() override;
        void Hide() override;

    private:
    
        math::Quad BoundingBox() const override;

        mono::InputSystem* m_input_system;
        game::UISystem* m_ui_system;

        UITextElement* m_quit_text;
        UITextElement* m_close_text;
        UIItemProxy m_quit_proxy;
        UIItemProxy m_close_proxy;

        UITextureElement* m_input_layout;

        uint32_t m_aborted_hash;
    };
}
