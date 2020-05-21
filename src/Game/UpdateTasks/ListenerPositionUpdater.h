
#pragma once

#include "IUpdatable.h"

namespace game
{
    class ListenerPositionUpdater : public mono::IUpdatable
    {
    public:
        void Update(const mono::UpdateContext& update_context) override;
    };
}
