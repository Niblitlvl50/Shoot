
#pragma once

#include "MonoFwd.h"
#include "IUpdatable.h"
#include "Physics/PhysicsFwd.h"

namespace game
{
    class GravityUpdater : public mono::IUpdatable
    {
    public:
        GravityUpdater(mono::IEntity* moon1, mono::IEntity* moon2);

        void Update(const mono::UpdateContext& update_context) override;
        void GravityFunc(mono::IBody* body);

    private:
        mono::IEntity* mMoon1;
        mono::IEntity* mMoon2;
        unsigned int mElapsedTime;
    };
}
