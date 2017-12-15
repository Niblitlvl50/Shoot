
#pragma once

#include "Zone/ZoneBase.h"
#include "EventHandler/EventToken.h"
#include "Events/EventFwd.h"

namespace game
{
    class TitleScreen : public mono::ZoneBase
    {
    public:

        TitleScreen(mono::EventHandler& event_handler);
        ~TitleScreen();
        
        bool OnKeyUp(const event::KeyUpEvent& event);

        void OnLoad(mono::ICameraPtr& camera) override;
        void OnUnload() override;

        mono::EventHandler& m_event_handler;
        mono::EventToken<event::KeyUpEvent> m_key_token;

        std::vector<struct MoveActionContext> m_move_contexts;
    };
}
