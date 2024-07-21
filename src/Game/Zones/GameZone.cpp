
#include "GameZone.h"
#include "Zones/ZoneFlow.h"
#include "GameCamera/CameraSystem.h"

#include "Rendering/RenderSystem.h"
#include "Rendering/ScreenFadeDrawer.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Sprite/SpriteBatchDrawer.h"
#include "Rendering/Text/TextSystem.h"
#include "Rendering/Text/TextBatchDrawer.h"
#include "Rendering/Lights/LightSystem.h"
#include "Rendering/Lights/LightSystemDrawer.h"

#include "Physics/PhysicsSystem.h"
#include "Physics/PhysicsDebugDrawer.h"

#include "Particle/ParticleSystem.h"
#include "Particle/ParticleSystemDrawer.h"
#include "Paths/PathSystem.h"
#include "RoadSystem/RoadSystem.h"
#include "RoadSystem/RoadBatchDrawer.h"

#include "SystemContext.h"
#include "EntitySystem/EntitySystem.h"
#include "TransformSystem/TransformSystem.h"
#include "TransformSystem/TransformSystemDrawer.h"
#include "System/Hash.h"

#include "DamageSystem/HealthbarDrawer.h"
#include "Player/PlayerAuxiliaryDrawer.h"
#include "Hud/Debug/PhysicsStatsElement.h"
#include "Hud/Debug/ConsoleDrawer.h"
#include "Hud/Debug/ParticleStatusDrawer.h"

#include "Navigation/NavMeshVisualizer.h"
#include "Navigation/NavigationSystem.h"

#include "EntitySystem/IEntityManager.h"

#include "RenderLayers.h"
#include "DamageSystem/DamageSystem.h"
#include "WorldFile.h"
#include "Camera/ICamera.h"
#include "Debug/GameDebug.h"
#include "Debug/GameDebugDrawer.h"

#include "InteractionSystem/InteractionSystem.h"
#include "InteractionSystem/InteractionSystemDrawer.h"

#include "DialogSystem/DialogSystem.h"
#include "DialogSystem/DialogSystemDrawer.h"

#include "Entity/AnimationSystem.h"
#include "Entity/EntityLogicSystem.h"
#include "Entity/EntityObjectDrawer.h"
#include "Entity/TargetSystem.h"
#include "Entity/TargetSystemDrawer.h"
#include "Mission/MissionSystem.h"
#include "Mission/MissionSystemDrawer.h"
#include "TriggerSystem/TriggerSystem.h"
#include "TriggerSystem/TriggerDebugDrawer.h"
#include "SpawnSystem/SpawnSystem.h"
#include "SpawnSystem/SpawnSystemDrawer.h"
#include "World/WorldBoundsSystem.h"
#include "World/WorldBoundsDrawer.h"
#include "World/RegionDrawer.h"
#include "World/RegionSystem.h"
#include "World/WorldEntityTrackingSystem.h"
#include "World/WorldEntityTrackingDrawer.h"
#include "UI/UISystem.h"
#include "UI/UISystemDrawer.h"

#include "ImGuiImpl/ImGuiInputHandler.h"

#include "GameMode/IGameMode.h"
#include "GameMode/GameModeFactory.h"
#include "Sound/SoundSystem.h"

using namespace game;

GameZone::GameZone(const ZoneCreationContext& context)
    : m_system_context(context.system_context)
    , m_event_handler(context.event_handler)
    , m_world_file(context.zone_filename)
{ }

GameZone::~GameZone()
{ }

void GameZone::OnLoad(mono::ICamera* camera, mono::IRenderer* renderer)
{
    SetLastLightingLayer(LayerId::GAMEOBJECTS_UI);

    mono::EntitySystem* entity_system = m_system_context->GetSystem<mono::EntitySystem>();
    entity_system->PushEntityStackRecord(m_world_file);

    mono::TransformSystem* transform_system = m_system_context->GetSystem<mono::TransformSystem>();
    mono::RenderSystem* render_system = m_system_context->GetSystem<mono::RenderSystem>();
    mono::PhysicsSystem* physics_system = m_system_context->GetSystem<mono::PhysicsSystem>();
    mono::SpriteSystem* sprite_system = m_system_context->GetSystem<mono::SpriteSystem>();
    mono::TextSystem* text_system = m_system_context->GetSystem<mono::TextSystem>();
    mono::ParticleSystem* particle_system = m_system_context->GetSystem<mono::ParticleSystem>();
    mono::LightSystem* light_system = m_system_context->GetSystem<mono::LightSystem>();
    mono::PathSystem* path_system = m_system_context->GetSystem<mono::PathSystem>();
    mono::RoadSystem* road_system = m_system_context->GetSystem<mono::RoadSystem>();

    game::AnimationSystem* animation_system = m_system_context->GetSystem<game::AnimationSystem>();
    game::CameraSystem* camera_system = m_system_context->GetSystem<CameraSystem>();
    game::DamageSystem* damage_system = m_system_context->GetSystem<DamageSystem>();
    game::TriggerSystem* trigger_system = m_system_context->GetSystem<TriggerSystem>();
    game::InteractionSystem* interaction_system = m_system_context->GetSystem<InteractionSystem>();
    game::SpawnSystem* spawn_system = m_system_context->GetSystem<SpawnSystem>();
    game::DialogSystem* dialog_system = m_system_context->GetSystem<DialogSystem>();
    game::WorldBoundsSystem* world_bounds_system = m_system_context->GetSystem<WorldBoundsSystem>();
    game::WorldEntityTrackingSystem* entity_tracking_system = m_system_context->GetSystem<WorldEntityTrackingSystem>();
    game::RegionSystem* region_system = m_system_context->GetSystem<RegionSystem>();
    game::UISystem* ui_system = m_system_context->GetSystem<UISystem>();
    game::TargetSystem* target_system = m_system_context->GetSystem<TargetSystem>();
    game::EntityLogicSystem* logic_system = m_system_context->GetSystem<EntityLogicSystem>();
    game::MissionSystem* mission_system = m_system_context->GetSystem<game::MissionSystem>();

    m_leveldata = ReadWorldComponentObjects(m_world_file, entity_system, nullptr);
    const game::LevelMetadata& metadata = m_leveldata.metadata;

    camera->SetPosition(metadata.camera_position);
    camera->SetViewportSize(metadata.camera_size);
    renderer->SetClearColor(metadata.background_color);
    renderer->SetAmbientShade(metadata.ambient_shade);

    SoundSystem* sound_system = m_system_context->GetSystem<SoundSystem>();
    sound_system->PlayBackgroundMusic(metadata.background_music, SoundTransition::CrossFade);

    // Nav mesh
    game::NavigationSystem* navigation_system = m_system_context->GetSystem<game::NavigationSystem>();
    navigation_system->SetupNavmesh(
        metadata.navmesh_start, metadata.navmesh_end, metadata.navmesh_density, physics_system->GetSpace());

    m_debug_input = std::make_unique<ImGuiInputHandler>(*m_event_handler);

    // Background
    AddDrawable(new game::WorldBoundsDrawer(transform_system, world_bounds_system, PolygonDrawLayer::PRE_GAMEOBJECTS), LayerId::BACKGROUND);
    AddDrawable(new mono::RoadBatchDrawer(road_system, path_system, transform_system), LayerId::BACKGROUND);

    // Pre Game Objects
    AddDrawable(new mono::ParticleSystemDrawer(particle_system, transform_system, mono::ParticleDrawLayer::PRE_GAMEOBJECTS), LayerId::PRE_GAMEOBJECTS);

    // Game Objects
    AddDrawable(new mono::SpriteBatchDrawer(transform_system, sprite_system, render_system), LayerId::GAMEOBJECTS);
    AddDrawable(new game::EntityObjectDrawer(logic_system), LayerId::GAMEOBJECTS);
    AddDrawable(new mono::LightSystemDrawer(light_system, transform_system), LayerId::GAMEOBJECTS);

    // Post Game Objects
    AddDrawable(new game::WorldBoundsDrawer(transform_system, world_bounds_system, PolygonDrawLayer::POST_GAMEOBJECTS), LayerId::POST_GAMEOBJECTS);
    AddDrawable(new mono::TextBatchDrawer(text_system, transform_system), LayerId::POST_GAMEOBJECTS);
    AddDrawable(new mono::ParticleSystemDrawer(particle_system, transform_system, mono::ParticleDrawLayer::POST_GAMEOBJECTS), LayerId::POST_GAMEOBJECTS);
    AddDrawable(new game::SpawnSystemDrawer(spawn_system, transform_system, particle_system, entity_system), LayerId::POST_GAMEOBJECTS);

    // Game Objects UI
    AddDrawable(new game::PlayerAuxiliaryDrawer(camera_system, transform_system), LayerId::GAMEOBJECTS_UI);
    m_healthbar_drawer = new game::HealthbarDrawer(damage_system, animation_system, text_system, transform_system, entity_system);
    AddUpdatableDrawable(m_healthbar_drawer, LayerId::GAMEOBJECTS_UI);
    AddDrawable(new game::InteractionSystemDrawer(interaction_system, sprite_system, transform_system, entity_system), LayerId::GAMEOBJECTS_UI);

    AddDrawable(new game::UISystemDrawer(ui_system, transform_system), LayerId::UI_OVERLAY);
    AddDrawable(new mono::ScreenFadeDrawer(render_system), LayerId::UI_OVERLAY);

    AddDrawable(new WorldEntityTrackingDrawer(entity_tracking_system, transform_system), LayerId::UI);
    AddDrawable(new game::DialogSystemDrawer(dialog_system, transform_system), LayerId::UI);
    m_region_ui = new RegionDrawer(region_system);
    AddUpdatableDrawable(m_region_ui, LayerId::UI);

    m_mission_ui = new MissionSystemDrawer(mission_system);
    AddUpdatableDrawable(m_mission_ui, LayerId::UI);

    // Debug
    AddDrawable(new PhysicsStatsElement(physics_system), LayerId::UI);
    AddDrawable(new ParticleStatusDrawer(particle_system), LayerId::UI);
    AddDrawable(new NavmeshVisualizer(navigation_system, *m_event_handler), LayerId::UI);
    AddDrawable(new mono::TransformSystemDrawer(g_draw_transformsystem, transform_system), LayerId::UI);
    AddDrawable(new mono::PhysicsDebugDrawer(
        g_draw_physics, g_interact_physics, g_body_introspection, g_draw_physics_subcomponents, physics_system, m_event_handler), LayerId::UI);
    AddDrawable(new TriggerDebugDrawer(g_draw_triggers, trigger_system, transform_system), LayerId::UI);
    AddDrawable(new TargetSystemDrawer(g_draw_targets, target_system), LayerId::UI);
    AddDrawable(new GameDebugDrawer(), LayerId::UI_DEBUG);

    m_debug_updater = new DebugUpdater(m_system_context, m_event_handler, renderer);
    AddUpdatableDrawable(m_debug_updater, LayerId::UI);

    // Game Mode
    m_game_mode = CreateGameMode();
    m_game_mode->Begin(this, renderer, m_system_context, m_event_handler, metadata);
    AddUpdatable(m_game_mode.get());
}

int GameZone::OnUnload()
{
    RemoveUpdatable(m_game_mode.get());
    const int game_mode_result = m_game_mode->End(this);
    m_game_mode = nullptr;

    RemoveUpdatableDrawable(m_healthbar_drawer);
    delete m_healthbar_drawer;
    m_healthbar_drawer = nullptr;

    RemoveUpdatableDrawable(m_region_ui);
    delete m_region_ui;
    m_region_ui = nullptr;

    RemoveUpdatableDrawable(m_mission_ui);
    delete m_mission_ui;
    m_mission_ui = nullptr;

    RemoveUpdatableDrawable(m_debug_updater);
    delete m_debug_updater;
    m_debug_updater = nullptr;

    ZoneBase::OnUnload();

    mono::EntitySystem* entity_system = m_system_context->GetSystem<mono::EntitySystem>();
    entity_system->PopEntityStackRecord();

    return game_mode_result;
}

std::unique_ptr<IGameMode> GameZone::CreateGameMode()
{
    const uint32_t game_mode_hash = hash::Hash(m_leveldata.metadata.level_game_mode.c_str());
    return game::GameModeFactory::CreateGameMode(game_mode_hash);
}
