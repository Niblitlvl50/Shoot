
#pragma once

#include "GameZone.h"

namespace game
{
    class EmptyZone : public GameZone
    {
    public:

        EmptyZone(const ZoneCreationContext& context);
        ~EmptyZone();

        void OnLoad(mono::ICamera* camera) override;
        int OnUnload() override;

        mono::EventHandler* m_event_handler;
        std::unique_ptr<class INetworkPipe> m_network_pipe;
        std::unique_ptr<class PlayerDaemon> m_player_daemon;
    };
}
