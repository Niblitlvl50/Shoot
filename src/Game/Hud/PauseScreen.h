
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
            class UISystem* ui_system);

        void ShowAt(const math::Vector& position);
        void Show() override;
        void Hide() override;

        mono::TransformSystem* m_transform_system;
        mono::InputSystem* m_input_system;
        mono::IEntityManager* m_entity_manager;
        game::UISystem* m_ui_system;

        UITextElement* m_exit_text;
        UIItemProxy m_proxy;

        std::vector<mono::Entity> m_entities;
    };
}
