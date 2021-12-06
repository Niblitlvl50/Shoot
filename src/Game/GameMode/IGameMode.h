
#pragma once

#include "MonoFwd.h"
#include "IUpdatable.h"

namespace game
{
    class IGameMode : public mono::IUpdatable
    {
    public:

        virtual void Begin(mono::IZone* zone, mono::SystemContext* system_context, mono::EventHandler* event_handler) = 0;
        virtual int End(mono::IZone* zone) = 0;
    };
}
