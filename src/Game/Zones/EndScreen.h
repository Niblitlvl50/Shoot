
#pragma once

#include "GameZone.h"

namespace game
{
    class EndScreen : public GameZone
    {
    public:

        EndScreen(const ZoneCreationContext& context);

        void OnLoad(mono::ICamera* camera, mono::IRenderer* renderer) override;
        int OnUnload() override;
    };
}
