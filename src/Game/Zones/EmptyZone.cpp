
#include "EmptyZone.h"

#include "Player/PlayerInfo.h"
#include "Network/INetworkPipe.h"
#include "RenderLayers.h"
#include "Player/PlayerDaemon.h"

#include "EntitySystem/EntitySystem.h"
#include "SystemContext.h"

using namespace game;

namespace
{
    class NullPipe : public INetworkPipe
    {
    public:

        void SendMessage(const NetworkMessage& message) override
        { }
        void SendMessageTo(const NetworkMessage& message, const network::Address& address) override
        { }
        ConnectionInfo GetConnectionInfo() const override
        {
            return ConnectionInfo();
        }
    };
}

EmptyZone::EmptyZone(const ZoneCreationContext& context)
    : GameZone(context, "res/worlds/empty_world.components")
    , m_event_handler(context.event_handler)
{ }

EmptyZone::~EmptyZone()
{ }

void EmptyZone::OnLoad(mono::ICamera* camera, mono::IRenderer* renderer)
{
    GameZone::OnLoad(camera, renderer);

    mono::EntitySystem* entity_system = m_system_context->GetSystem<mono::EntitySystem>();

    // Player
    m_network_pipe = std::make_unique<NullPipe>();
    m_player_daemon =
        std::make_unique<PlayerDaemon>(m_network_pipe.get(), entity_system, m_system_context, m_event_handler, m_leveldata.metadata.player_spawn_point);
}

int EmptyZone::OnUnload()
{
    GameZone::OnUnload();
    return 0;
}
