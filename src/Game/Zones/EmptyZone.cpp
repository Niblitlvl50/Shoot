
#include "EmptyZone.h"

#include "AIKnowledge.h"
#include "Hud/Overlay.h"
#include "Hud/PlayerUIElement.h"
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
        std::make_unique<PlayerDaemon>(m_network_pipe.get(), entity_system, m_system_context, m_event_handler, m_player_spawn_point);

    // Ui
    UIOverlayDrawer* hud_overlay = new UIOverlayDrawer();
    hud_overlay->AddChild(new PlayerUIElement(g_player_one, math::Vector(0.0f, 0.0f), math::Vector(-100.0f, 0.0f)));
    hud_overlay->AddChild(new PlayerUIElement(g_player_two, math::Vector(277.0f, 0.0f), math::Vector(320.0f, 0.0f)));
    AddEntity(hud_overlay, LayerId::UI);
}

int EmptyZone::OnUnload()
{
    GameZone::OnUnload();
    return 0;
}
