
#pragma once

#include "UIElements.h"
#include "Input/InputInterfaces.h"

namespace game
{
    class PauseScreen : public game::UIOverlay //, public mono::
    {
    public:

        PauseScreen(mono::InputSystem* input_system);

        mono::InputSystem* m_input_system;
    };
}
