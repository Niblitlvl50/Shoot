
#pragma once

#include "MonoFwd.h"
#include "MonoPtrFwd.h"
#include "Physics/CMFwd.h"

#include "IUpdatable.h"

namespace game
{
    class GravityUpdater : public mono::IUpdatable
    {
    public:
        GravityUpdater(/*mono::IPhysicsZone* zone, */const mono::IEntityPtr& moon1, const mono::IEntityPtr& moon2);

        void doUpdate(const mono::UpdateContext& update_context) override;
        void GravityFunc(mono::IBody* body);

    private:
        //mono::IPhysicsZone* mZone;
        mono::IEntityPtr mMoon1;
        mono::IEntityPtr mMoon2;
        unsigned int mElapsedTime;
    };
}
