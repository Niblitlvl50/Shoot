
#pragma once

#include "ZoneCreationContext.h"
#include "Zone/ZoneBase.h"
#include "GameZone.h"
#include "EventHandler/EventToken.h"
#include "Events/EventFwd.h"

#include <memory>
#include <cstdint>

namespace game
{
    class SetupGameScreen : public GameZone
    {
    public:

        SetupGameScreen(const ZoneCreationContext& context);
        ~SetupGameScreen();
        
        void OnLoad(mono::ICamera* camera, mono::IRenderer* renderer) override;
        int OnUnload() override;

        void Continue();
        void Remote();
        void Quit();

        mono::EventHandler& m_event_handler;
        mono::SystemContext* m_system_context;
        mono::EventToken<event::KeyUpEvent> m_key_token;
        int m_exit_zone;

        std::unique_ptr<class ScreenSparkles> m_sparkles;
        std::vector<uint32_t> m_loaded_entities;
    };
}
