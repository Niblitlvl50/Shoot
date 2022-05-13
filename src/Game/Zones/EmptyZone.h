
#pragma once

#include "SystemTestZone.h"

namespace game
{
    class EmptyZone : public SystemTestZone
    {
    public:

        EmptyZone(const ZoneCreationContext& context);
        ~EmptyZone();

        void OnLoad(mono::ICamera* camera, mono::IRenderer* renderer) override;
        int OnUnload() override;

        mono::EventHandler* m_event_handler;
        std::unique_ptr<class INetworkPipe> m_network_pipe;
        std::unique_ptr<class PlayerDaemon> m_player_daemon;
    };
}
