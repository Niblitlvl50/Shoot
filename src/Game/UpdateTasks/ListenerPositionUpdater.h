
#pragma once

#include "IUpdatable.h"

namespace game
{
    class ListenerPositionUpdater : public mono::IUpdatable
    {
    public:
        void doUpdate(unsigned int delta) override;
    };
}
