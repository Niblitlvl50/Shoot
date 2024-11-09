
#include "ServerGameZone.h"
#include "GameMode/PacketDeliveryGameMode.h"

#include "EntitySystem/Entity.h"
#include "EntitySystem/EntitySystem.h"
#include "Physics/PhysicsSystem.h"
#include "Physics/PhysicsSpace.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "TransformSystem/TransformSystem.h"
#include "SystemContext.h"

#include "Hud/Debug/NetworkStatusDrawer.h"
#include "Hud/Debug/ClientViewportVisualizer.h"

#include "Network/ServerManager.h"
#include "Network/ServerReplicator.h"

#include "DamageSystem/DamageSystem.h"

#include "RenderLayers.h"

using namespace game;

ServerGameZone::ServerGameZone(const ZoneCreationContext& context)
    : GameZone(context)
    , m_system_context(context.system_context)
    , m_event_handler(context.event_handler)
    , m_game_config(*context.game_config)
{ }

ServerGameZone::~ServerGameZone()
{ }

void ServerGameZone::OnLoad(mono::ICamera* camera, mono::IRenderer* renderer)
{
    GameZone::OnLoad(camera, renderer);

    mono::EntitySystem* entity_system = m_system_context->GetSystem<mono::EntitySystem>();
    mono::TransformSystem* transform_system = m_system_context->GetSystem<mono::TransformSystem>();
    mono::SpriteSystem* sprite_system = m_system_context->GetSystem<mono::SpriteSystem>();
    mono::PhysicsSystem* physics_system = m_system_context->GetSystem<mono::PhysicsSystem>();
    physics_system->GetSpace()->SetDamping(0.01f);

    game::DamageSystem* damage_system = m_system_context->GetSystem<game::DamageSystem>();
    game::ServerManager* server_manager = m_system_context->GetSystem<game::ServerManager>();
    //server_manager->StartServer();

    ServerReplicator* server_replicator = new ServerReplicator(
        m_event_handler,
        entity_system,
        transform_system,
        sprite_system,
        damage_system,
        server_manager,
        m_leveldata.metadata,
        m_game_config.server_replication_interval);
    AddUpdatable(server_replicator);

    // Debug
    AddDrawable(new ClientViewportVisualizer(server_manager->GetConnectedClients()), LayerId::UI);
    AddDrawable(new NetworkStatusDrawer(server_manager), LayerId::UI);
}

int ServerGameZone::OnUnload()
{
    game::ServerManager* server_manager = m_system_context->GetSystem<game::ServerManager>();
    server_manager->QuitServer();

    return GameZone::OnUnload();
}
