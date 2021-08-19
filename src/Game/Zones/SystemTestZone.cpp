
#include "SystemTestZone.h"
#include "ZoneFlow.h"

#include "EntitySystem/Entity.h"
#include "EntitySystem/EntitySystem.h"
#include "Events/QuitEvent.h"
#include "EventHandler/EventHandler.h"
#include "Events/GameEvents.h"
#include "Events/GameEventFuncFwd.h"
#include "Physics/PhysicsSystem.h"
#include "Physics/PhysicsSpace.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "SystemContext.h"
#include "TransformSystem/TransformSystem.h"
#include "System/Hash.h"
#include "Particle/ParticleSystem.h"

#include "Hud/PlayerDeathScreen.h"
#include "Hud/PlayerUIElement.h"
#include "Hud/RegionDrawer.h"
#include "Hud/Debug/NetworkStatusDrawer.h"
#include "Hud/Debug/ClientViewportVisualizer.h"

#include "Navigation/NavmeshFactory.h"
#include "Navigation/NavMeshVisualizer.h"

#include "Network/ServerManager.h"
#include "Network/ServerReplicator.h"

#include "Factories.h"
#include "RenderLayers.h"
#include "Player/PlayerDaemon.h"
#include "TriggerSystem/TriggerSystem.h"
#include "DamageSystem.h"

#include "World/FogOverlay.h"
#include "Effects/AngelDust.h"

#include "System/System.h"

namespace
{
    const uint32_t level_completed_hash = hash::Hash("level_completed");

    mono::EventResult GameOverAndQuit(int& next_zone, mono::EventHandler* event_handler)
    {
        next_zone = game::ZoneFlow::GAME_OVER_SCREEN;

        const auto send_quit = [](void* data) {
            mono::EventHandler* event_handler = static_cast<mono::EventHandler*>(data);
            event_handler->DispatchEvent(event::QuitEvent());
        };
        System::CreateTimer(1000, System::TimerProperties::AUTO_DELETE | System::TimerProperties::ONE_SHOT, send_quit, event_handler);

        return mono::EventResult::PASS_ON;
    }

    void GameCompleted(int32_t trigger_id, int& next_zone, mono::EventHandler* event_handler)
    {
        next_zone = game::ZoneFlow::END_SCREEN;
        event_handler->DispatchEvent(event::QuitEvent());
    }
}

using namespace game;

SystemTestZone::SystemTestZone(const ZoneCreationContext& context)
    : GameZone(context, "res/worlds/world.components")
    , m_system_context(context.system_context)
    , m_event_handler(context.event_handler)
    , m_game_config(*context.game_config)
    , m_next_zone(TITLE_SCREEN)
{
    const GameOverFunc on_game_over = std::bind(GameOverAndQuit, std::ref(m_next_zone), m_event_handler);
    m_gameover_token = m_event_handler->AddListener(on_game_over);
}

SystemTestZone::~SystemTestZone()
{
    m_event_handler->RemoveListener(m_gameover_token);
}

void SystemTestZone::OnLoad(mono::ICamera* camera, mono::IRenderer* renderer)
{
    GameZone::OnLoad(camera, renderer);

    //mono::IShader* screen_shader = renderer->GetScreenShader();
    //renderer->UseShader(screen_shader);
    //mono::ScreenShader::FadeCorners(screen_shader, true);

    mono::EntitySystem* entity_system = m_system_context->GetSystem<mono::EntitySystem>();
    mono::TransformSystem* transform_system = m_system_context->GetSystem<mono::TransformSystem>();
    mono::SpriteSystem* sprite_system = m_system_context->GetSystem<mono::SpriteSystem>();
    mono::ParticleSystem* particle_system = m_system_context->GetSystem<mono::ParticleSystem>();
    mono::PhysicsSystem* physics_system = m_system_context->GetSystem<mono::PhysicsSystem>();
    physics_system->GetSpace()->SetDamping(0.01f);

    game::DamageSystem* damage_system = m_system_context->GetSystem<game::DamageSystem>();
    game::TriggerSystem* trigger_system = m_system_context->GetSystem<game::TriggerSystem>();
    game::ServerManager* server_manager = m_system_context->GetSystem<game::ServerManager>();
    server_manager->StartServer();

    using namespace std::placeholders;
    m_level_completed_trigger = trigger_system->RegisterTriggerCallback(
        level_completed_hash,
        std::bind(GameCompleted, _1, std::ref(m_next_zone), m_event_handler),
        mono::INVALID_ID);

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

    // Player
    m_player_daemon = std::make_unique<PlayerDaemon>(
        server_manager, entity_system, m_system_context, m_event_handler, m_leveldata.metadata.player_spawn_point);
    
    m_player_death_screen = std::make_unique<PlayerDeathScreen>(game::g_players[0], m_event_handler);
    m_player_ui = std::make_unique<PlayerUIElement>(game::g_players[0]);
    //m_fog = std::make_unique<FogOverlay>();

    m_angeldust_effect = std::make_unique<AngelDust>(particle_system, entity_system, math::Quad(-50.0f, -50.0f, 50.0f, 50.0f));

    AddUpdatableDrawable(m_player_death_screen.get(), LayerId::UI);
    AddUpdatableDrawable(m_player_ui.get(), LayerId::UI);
    AddUpdatableDrawable(new RegionDrawer(trigger_system), LayerId::UI);
    //AddUpdatableDrawable(m_fog.get(), LayerId::FOG);

    // Nav mesh
    std::vector<ExcludeZone> exclude_zones;
    m_navmesh.points = game::GenerateMeshPoints(math::Vector(-100, -50), 150, 100, 3, exclude_zones);
    m_navmesh.nodes = game::GenerateMeshNodes(m_navmesh.points, 5, exclude_zones);

    // Debug
    AddDrawable(new ClientViewportVisualizer(server_manager->GetConnectedClients()), LayerId::UI);
    AddDrawable(new NetworkStatusDrawer(server_manager), LayerId::UI);
}

int SystemTestZone::OnUnload()
{
    GameZone::OnUnload();

    game::ServerManager* server_manager = m_system_context->GetSystem<game::ServerManager>();
    server_manager->QuitServer();

    TriggerSystem* trigger_system = m_system_context->GetSystem<TriggerSystem>();
    trigger_system->RemoveTriggerCallback(level_completed_hash, m_level_completed_trigger, 0);

    RemoveUpdatableDrawable(m_player_death_screen.get());
    RemoveUpdatableDrawable(m_player_ui.get());
    //RemoveUpdatableDrawable(m_fog.get());

    return m_next_zone;
}
