
#pragma once

#include "ZoneCreationContext.h"
#include "GameZone.h"
#include "EventHandler/EventToken.h"
#include "Events/EventFwd.h"

namespace game
{
    class TitleScreen : public GameZone
    {
    public:

        TitleScreen(const ZoneCreationContext& context);
        ~TitleScreen();
        
        void OnLoad(mono::ICamera* camera, mono::IRenderer* renderer) override;
        int OnUnload() override;

        void Continue();
        void Quit();

        mono::EventHandler& m_event_handler;
        mono::EventToken<event::KeyUpEvent> m_key_token;
        int m_exit_zone = 0;
    };
}
