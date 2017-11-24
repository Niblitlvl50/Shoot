
#pragma once

#include "IUpdatable.h"
#include "MonoFwd.h"
#include <vector>

namespace game
{
    class DamageController;
    struct Healthbar;

    class HealthbarUpdater : public mono::IUpdatable
    {
    public:

        HealthbarUpdater(
            std::vector<Healthbar>& healthbars,
            const DamageController& damage_controller,
            const mono::IPhysicsZone& zone);

        void doUpdate(unsigned int delta) override;

    private:

        std::vector<Healthbar>& m_healthbars;
        const DamageController& m_damageController;
        const mono::IPhysicsZone& m_zone;
    };
}
