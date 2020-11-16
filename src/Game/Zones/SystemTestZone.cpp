
#include "SystemTestZone.h"

#include "EntitySystem/EntitySystem.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "SystemContext.h"
#include "TransformSystem/TransformSystem.h"

#include "Hud/Overlay.h"
#include "Hud/PlayerUIElement.h"
#include "Hud/Debug/NetworkStatusDrawer.h"
#include "Hud/Debug/ClientViewportVisualizer.h"

#include "Navigation/NavmeshFactory.h"
#include "Navigation/NavMeshVisualizer.h"

#include "Network/ServerManager.h"
#include "Network/ServerReplicator.h"

#include "UpdateTasks/ListenerPositionUpdater.h"
#include "EntitySystem/IEntityManager.h"
#include "Factories.h"
#include "RenderLayers.h"
#include "Player/PlayerDaemon.h"

using namespace game;

namespace
{
    class SyncPoint : public mono::IUpdatable
    {
        void Update(const mono::UpdateContext& update_context)
        {
            g_entity_manager->Sync();
        }
    };
}

SystemTestZone::SystemTestZone(const ZoneCreationContext& context)
    : GameZone(context, "res/world.components")
    , m_system_context(context.system_context)
    , m_event_handler(context.event_handler)
    , m_game_config(*context.game_config)
{ }

SystemTestZone::~SystemTestZone()
{ }

void SystemTestZone::OnLoad(mono::ICamera* camera)
{
    GameZone::OnLoad(camera);

    mono::EntitySystem* entity_system = m_system_context->GetSystem<mono::EntitySystem>();
    mono::TransformSystem* transform_system = m_system_context->GetSystem<mono::TransformSystem>();
    mono::SpriteSystem* sprite_system = m_system_context->GetSystem<mono::SpriteSystem>();

    // Network and syncing should be done first in the frame.
    m_server_manager = std::make_unique<ServerManager>(m_event_handler, &m_game_config);
    AddUpdatable(m_server_manager.get());
    AddUpdatable(
        new ServerReplicator(
            entity_system,
            transform_system,
            sprite_system,
            g_entity_manager,
            m_server_manager.get(),
            m_game_config.server_replication_interval)
        );
    AddUpdatable(new SyncPoint()); // this should probably go last or something...
    AddUpdatable(new ListenerPositionUpdater());

    // Player
    m_player_daemon = std::make_unique<PlayerDaemon>(m_server_manager.get(), m_system_context, *m_event_handler);

    // Nav mesh
    std::vector<ExcludeZone> exclude_zones;
    m_navmesh.points = game::GenerateMeshPoints(math::Vector(-100, -50), 150, 100, 3, exclude_zones);
    m_navmesh.nodes = game::GenerateMeshNodes(m_navmesh.points, 5, exclude_zones);

    // Ui
    UIOverlayDrawer* hud_overlay = new UIOverlayDrawer();
    hud_overlay->AddChild(new PlayerUIElement(g_player_one, math::Vector(0.0f, 0.0f), math::Vector(-100.0f, 0.0f)));
    hud_overlay->AddChild(new PlayerUIElement(g_player_two, math::Vector(277.0f, 0.0f), math::Vector(320.0f, 0.0f)));
    AddEntity(hud_overlay, LayerId::UI);

    // Debug
    UIOverlayDrawer* debug_hud_overlay = new UIOverlayDrawer();
    debug_hud_overlay->AddChild(new NetworkStatusDrawer(math::Vector(2.0f, 190.0f), m_server_manager.get()));
    AddEntity(debug_hud_overlay, LayerId::UI);
    AddDrawable(new ClientViewportVisualizer(m_server_manager->GetConnectedClients()), LayerId::UI);
}

int SystemTestZone::OnUnload()
{
    GameZone::OnUnload();
    RemoveUpdatable(m_server_manager.get());
    return 0;
}
