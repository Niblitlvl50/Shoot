
#pragma once

#include "UIElements.h"

namespace game
{
    class PauseScreen : public game::UIElement
    {
    public:

        PauseScreen(mono::InputSystem* input_system, mono::IEntityManager* entity_manager, class UISystem* ui_system);

        void ShowAt(const math::Vector& position);
        void Show() override;
        void Hide() override;

        mono::InputSystem* m_input_system;
        mono::IEntityManager* m_entity_manager;
        game::UISystem* m_ui_system;
    };
}
