
#pragma once

#include "MonoFwd.h"
#include "Math/MathFwd.h"
#include "Physics/CMFwd.h"

namespace game
{
    class HomingBehaviour
    {
    public:

        HomingBehaviour(class Enemy* enemy, mono::EventHandler& event_handler);
        ~HomingBehaviour();

        void SetHomingPosition(const math::Vector& position);

    private:

        mono::EventHandler& m_event_handler;
        mono::IBodyPtr m_control_body;
        mono::IConstraintPtr m_spring;
    };
}
