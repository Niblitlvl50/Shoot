
#pragma once

#include "MonoFwd.h"

namespace game
{
    class StaggerBehaviour
    {
    public:

        StaggerBehaviour();
        StaggerBehaviour(float stagger_chance_fraction, float stagger_duration);
        void Update(const mono::UpdateContext& update_context);

        bool IsStaggering() const;
        bool TestForStaggering();

        void SetChanceAndDuration(float chance_fraction, float duration_s);
        
    private:

        float m_stagger_chance;
        float m_stagger_duration;
        float m_stagger_timer;
        bool m_staggering;
    };
}
