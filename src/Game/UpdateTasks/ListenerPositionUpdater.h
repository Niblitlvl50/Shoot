
#pragma once

#include "IUpdatable.h"

namespace game
{
    class ListenerPositionUpdater : public mono::IUpdatable
    {
    public:
        void doUpdate(const mono::UpdateContext& update_context) override;
    };
}
