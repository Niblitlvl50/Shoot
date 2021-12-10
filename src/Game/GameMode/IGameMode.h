
#pragma once

#include "MonoFwd.h"
#include "Math/MathFwd.h"
#include "Rendering/RenderFwd.h"
#include "IUpdatable.h"

namespace game
{
    class IGameMode : public mono::IUpdatable
    {
    public:

        virtual void Begin(
            mono::IZone* zone,
            mono::IRenderer* renderer,
            mono::SystemContext* system_context,
            mono::EventHandler* event_handler,
            const math::Vector& player_spawn) = 0;
        virtual int End(mono::IZone* zone) = 0;
    };
}
