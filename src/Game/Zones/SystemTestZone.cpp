
#include "SystemTestZone.h"
#include "ZoneFlow.h"
#include "GameMode/PacketDeliveryGameMode.h"

#include "EntitySystem/Entity.h"
#include "EntitySystem/EntitySystem.h"
#include "Physics/PhysicsSystem.h"
#include "Physics/PhysicsSpace.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "SystemContext.h"
#include "TransformSystem/TransformSystem.h"
#include "TriggerSystem/TriggerSystem.h"
#include "Particle/ParticleSystem.h"

#include "Hud/RegionDrawer.h"
#include "Hud/Debug/NetworkStatusDrawer.h"
#include "Hud/Debug/ClientViewportVisualizer.h"

#include "Network/ServerManager.h"
#include "Network/ServerReplicator.h"

#include "Factories.h"
#include "RenderLayers.h"
#include "DamageSystem.h"

#include "World/FogOverlay.h"

#include "System/System.h"


using namespace game;

SystemTestZone::SystemTestZone(const ZoneCreationContext& context, const char* zone_file)
    : GameZone(context, zone_file)
    , m_system_context(context.system_context)
    , m_event_handler(context.event_handler)
    , m_game_config(*context.game_config)
{ }

SystemTestZone::~SystemTestZone()
{ }

void SystemTestZone::OnLoad(mono::ICamera* camera, mono::IRenderer* renderer)
{
    GameZone::OnLoad(camera, renderer);

    //mono::IShader* screen_shader = renderer->GetScreenShader();
    //renderer->UseShader(screen_shader);
    //mono::ScreenShader::FadeCorners(screen_shader, true);

    mono::EntitySystem* entity_system = m_system_context->GetSystem<mono::EntitySystem>();
    mono::TransformSystem* transform_system = m_system_context->GetSystem<mono::TransformSystem>();
    mono::SpriteSystem* sprite_system = m_system_context->GetSystem<mono::SpriteSystem>();
    mono::PhysicsSystem* physics_system = m_system_context->GetSystem<mono::PhysicsSystem>();
    physics_system->GetSpace()->SetDamping(0.01f);

    game::DamageSystem* damage_system = m_system_context->GetSystem<game::DamageSystem>();
    game::TriggerSystem* trigger_system = m_system_context->GetSystem<game::TriggerSystem>();
    game::ServerManager* server_manager = m_system_context->GetSystem<game::ServerManager>();
    server_manager->StartServer();

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

    const std::vector<game::RegionDescription> regions = game::ParseRegionConfig("res/region_config.json");
    m_region_ui = std::make_unique<RegionDrawer>(regions, trigger_system);
    //m_fog = std::make_unique<FogOverlay>();

    AddUpdatableDrawable(m_region_ui.get(), LayerId::UI);
    //AddUpdatableDrawable(m_fog.get(), LayerId::FOG);

    // Debug
    AddDrawable(new ClientViewportVisualizer(server_manager->GetConnectedClients()), LayerId::UI);
    AddDrawable(new NetworkStatusDrawer(server_manager), LayerId::UI);
}

int SystemTestZone::OnUnload()
{
    game::ServerManager* server_manager = m_system_context->GetSystem<game::ServerManager>();
    server_manager->QuitServer();

    RemoveUpdatableDrawable(m_region_ui.get());
    //RemoveUpdatableDrawable(m_fog.get());

    return GameZone::OnUnload();
}

std::unique_ptr<IGameMode> SystemTestZone::CreateGameMode()
{
    return std::make_unique<PacketDeliveryGameMode>(math::ZeroVec);
}
