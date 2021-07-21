
#pragma once

#include "GameZone.h"
#include "MonoFwd.h"

namespace game
{
    class GameOverScreen : public game::GameZone
    {
    public:

        GameOverScreen(const ZoneCreationContext& context);
        void OnLoad(mono::ICamera* camera, mono::IRenderer* renderer) override;
        int OnUnload() override;

    private:

        mono::EventHandler* m_event_handler;
    };
}
