
#pragma once

#include "IUpdatable.h"
#include "MonoFwd.h"
#include <vector>

namespace game
{
    class PickupUpdater : public mono::IUpdatable
    {
    public:
        PickupUpdater(std::vector<struct Ammo>& pickups, mono::EventHandler& event_handler);
        void doUpdate(unsigned int delta) override;

    private:
        std::vector<struct Ammo>& m_pickups;
        mono::EventHandler& m_event_handler;
    };
}
