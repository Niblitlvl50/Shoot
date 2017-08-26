
#pragma once

#include "IUpdatable.h"

namespace game
{
    class ListenerPositionUpdater : public mono::IUpdatable
    {
    public:
        virtual void doUpdate(unsigned int delta);
    };
}
