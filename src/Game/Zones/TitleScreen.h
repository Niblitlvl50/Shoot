
#pragma once

#include "ZoneCreationContext.h"
#include "Zone/ZoneBase.h"
#include "EventHandler/EventToken.h"
#include "Events/EventFwd.h"

namespace game
{
    class TitleScreen : public mono::ZoneBase
    {
    public:

        TitleScreen(const ZoneCreationContext& context);
        ~TitleScreen();
        
        mono::EventResult OnKeyUp(const event::KeyUpEvent& event);

        void OnLoad(mono::ICamera* camera) override;
        int OnUnload() override;

        void Continue();
        void Remote();
        void Quit();

        mono::EventHandler& m_event_handler;
        mono::SystemContext* m_system_context;
        mono::EventToken<event::KeyUpEvent> m_key_token;
        std::vector<struct MoveActionContext> m_move_contexts;
        int m_exit_zone = 0;

        std::unique_ptr<class ScreenSparkles> m_sparkles;
    };
}
