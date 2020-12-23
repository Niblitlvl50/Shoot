
#include "SystemTestZone.h"
#include "ZoneFlow.h"

#include "EntitySystem/EntitySystem.h"
#include "Events/QuitEvent.h"
#include "EventHandler/EventHandler.h"
#include "Physics/PhysicsSystem.h"
#include "Physics/CMSpace.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Shader/ScreenShader.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "SystemContext.h"
#include "TransformSystem/TransformSystem.h"
#include "Util/Hash.h"

#include "Hud/Overlay.h"
#include "Hud/GameOverScreen.h"
#include "Hud/PlayerUIElement.h"
#include "Hud/Debug/NetworkStatusDrawer.h"
#include "Hud/Debug/ClientViewportVisualizer.h"

#include "Navigation/NavmeshFactory.h"
#include "Navigation/NavMeshVisualizer.h"

#include "Network/ServerManager.h"
#include "Network/ServerReplicator.h"

#include "UpdateTasks/ListenerPositionUpdater.h"
#include "Factories.h"
#include "RenderLayers.h"
#include "Player/PlayerDaemon.h"
#include "TriggerSystem.h"


namespace
{
    const uint32_t level_completed_hash = mono::Hash("level_completed");
}

using namespace game;

SystemTestZone::SystemTestZone(const ZoneCreationContext& context)
    : GameZone(context, "res/world.components")
    , m_system_context(context.system_context)
    , m_event_handler(context.event_handler)
    , m_game_config(*context.game_config)
    , m_next_zone(TITLE_SCREEN)
{ }

SystemTestZone::~SystemTestZone()
{ }

void SystemTestZone::OnLoad(mono::ICamera* camera, mono::IRenderer* renderer)
{
    GameZone::OnLoad(camera, renderer);

    mono::IShader* screen_shader = renderer->GetScreenShader();
    renderer->UseShader(screen_shader);
    mono::ScreenShader::FadeCorners(screen_shader, true);

    mono::EntitySystem* entity_system = m_system_context->GetSystem<mono::EntitySystem>();
    mono::TransformSystem* transform_system = m_system_context->GetSystem<mono::TransformSystem>();
    mono::SpriteSystem* sprite_system = m_system_context->GetSystem<mono::SpriteSystem>();
    mono::PhysicsSystem* physics_system = m_system_context->GetSystem<mono::PhysicsSystem>();
    physics_system->GetSpace()->SetDamping(0.01f);

    game::TriggerSystem* trigger_system = m_system_context->GetSystem<game::TriggerSystem>();

    const auto level_completed_func = [this](uint32_t trigger_id) {
        printf("Level completed, good job!\n");
        m_next_zone = END_SCREEN;
        m_event_handler->DispatchEvent(event::QuitEvent());
    };
    m_level_completed_trigger =
        trigger_system->RegisterTriggerCallback(level_completed_hash, level_completed_func, mono::INVALID_ID);

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
    AddUpdatable(new ListenerPositionUpdater());

    // Player
    m_player_daemon =
        std::make_unique<PlayerDaemon>(m_server_manager.get(), m_system_context, m_event_handler, m_player_spawn_point);

    // Nav mesh
    std::vector<ExcludeZone> exclude_zones;
    m_navmesh.points = game::GenerateMeshPoints(math::Vector(-100, -50), 150, 100, 3, exclude_zones);
    m_navmesh.nodes = game::GenerateMeshNodes(m_navmesh.points, 5, exclude_zones);

    // Ui
    UIOverlayDrawer* hud_overlay = new UIOverlayDrawer();
    hud_overlay->AddChild(new GameOverScreen(g_player_one, math::Vector(150.0f, 150.0f), math::Vector(400.0f, 150.0f), m_event_handler));
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
    TriggerSystem* trigger_system = m_system_context->GetSystem<TriggerSystem>();
    trigger_system->RemoveTriggerCallback(level_completed_hash, m_level_completed_trigger, 0);

    GameZone::OnUnload();
    RemoveUpdatable(m_server_manager.get());

    return m_next_zone;
}
