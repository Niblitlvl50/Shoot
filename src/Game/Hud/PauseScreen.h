
#pragma once

#include "UIElements.h"
#include "Input/InputInterfaces.h"

namespace game
{
    class UISystem;
    class CameraSystem;

    class PauseScreen : public game::UIElement //game::UIOverlay
    {
    public:

        PauseScreen(mono::InputSystem* input_system, mono::IEntityManager* entity_manager, game::UISystem* ui_system, game::CameraSystem* camera_system);

        void Show() override;
        void Hide() override;

        mono::InputSystem* m_input_system;
        mono::IEntityManager* m_entity_manager;
        game::UISystem* m_ui_system;
        game::CameraSystem* m_camera_system;
    };
}
