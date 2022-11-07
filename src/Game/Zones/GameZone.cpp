
#include "GameZone.h"
#include "Zones/ZoneFlow.h"

#include "Rendering/IRenderer.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Sprite/SpriteBatchDrawer.h"
#include "Rendering/Text/TextSystem.h"
#include "Rendering/Text/TextBatchDrawer.h"
#include "Rendering/Objects/StaticBackground.h"
#include "Rendering/Lights/LightSystem.h"
#include "Rendering/Lights/LightSystemDrawer.h"

#include "Physics/PhysicsSystem.h"
#include "Physics/PhysicsDebugDrawer.h"
#include "Physics/PhysicsSpace.h"

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

#include "Player/PlayerInfo.h"
#include "Hud/HealthbarDrawer.h"
#include "Hud/PlayerAuxiliaryDrawer.h"
#include "Hud/Debug/PhysicsStatsElement.h"
#include "Hud/Debug/ConsoleDrawer.h"
#include "Hud/Debug/ParticleStatusDrawer.h"

#include "Navigation/NavmeshFactory.h"
#include "Navigation/NavMeshVisualizer.h"

#include "EntitySystem/IEntityManager.h"

#include "RenderLayers.h"
#include "DamageSystem.h"
#include "WorldFile.h"
#include "Camera/ICamera.h"
#include "GameDebug.h"
#include "GameDebugDrawer.h"
#include "Factories.h"

#include "InteractionSystem/InteractionSystem.h"
#include "InteractionSystem/InteractionSystemDrawer.h"

#include "DialogSystem/DialogSystem.h"
#include "DialogSystem/DialogSystemDrawer.h"

#include "TriggerSystem/TriggerSystem.h"
#include "TriggerSystem/TriggerDebugDrawer.h"
#include "SpawnSystem/SpawnSystem.h"
#include "SpawnSystem/SpawnSystemDrawer.h"
#include "World/WorldBoundsSystem.h"
#include "World/WorldBoundsDrawer.h"
#include "World/RegionDrawer.h"
#include "World/RegionSystem.h"

#include "ImGuiImpl/ImGuiInputHandler.h"

#include "Util/Algorithm.h"
#include "CollisionConfiguration.h"

#include "GameMode/IGameMode.h"
#include "GameMode/GameModeFactory.h"
#include "Sound/SoundSystem.h"

namespace
{
    void SetupNavmesh(game::NavmeshContext& navmesh_context, const game::LevelMetadata& metadata, mono::PhysicsSpace* space)
    {
        navmesh_context.points = game::GenerateMeshPoints(metadata.navmesh_start, metadata.navmesh_end, metadata.navmesh_density);

        const auto remove_on_collision = [space](const math::Vector& point) {
            const mono::QueryResult query_result = space->QueryNearest(point, 0.0f, game::CollisionCategory::STATIC);
            return query_result.body != nullptr;
        };
        mono::remove_if(navmesh_context.points, remove_on_collision);

        const auto filter_connection_func = [space](const math::Vector& first, const math::Vector& second){
            const mono::QueryResult query_result = space->QueryFirst(first, second, game::CollisionCategory::STATIC);
            return query_result.body != nullptr;
        };

        navmesh_context.nodes = game::GenerateMeshNodes(navmesh_context.points, metadata.navmesh_density * 1.5f, filter_connection_func);
    }
}

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
    SetLastLightingLayer(LayerId::FOG);

    mono::EntitySystem* entity_system = m_system_context->GetSystem<mono::EntitySystem>();
    entity_system->PushEntityStackRecord(m_world_file);

    mono::TransformSystem* transform_system = m_system_context->GetSystem<mono::TransformSystem>();
    mono::PhysicsSystem* physics_system = m_system_context->GetSystem<mono::PhysicsSystem>();
    mono::SpriteSystem* sprite_system = m_system_context->GetSystem<mono::SpriteSystem>();
    mono::TextSystem* text_system = m_system_context->GetSystem<mono::TextSystem>();
    mono::ParticleSystem* particle_system = m_system_context->GetSystem<mono::ParticleSystem>();
    mono::LightSystem* light_system = m_system_context->GetSystem<mono::LightSystem>();
    mono::PathSystem* path_system = m_system_context->GetSystem<mono::PathSystem>();
    mono::RoadSystem* road_system = m_system_context->GetSystem<mono::RoadSystem>();
    DamageSystem* damage_system = m_system_context->GetSystem<DamageSystem>();
    TriggerSystem* trigger_system = m_system_context->GetSystem<TriggerSystem>();
    InteractionSystem* interaction_system = m_system_context->GetSystem<InteractionSystem>();
    SpawnSystem* spawn_system = m_system_context->GetSystem<SpawnSystem>();
    DialogSystem* dialog_system = m_system_context->GetSystem<DialogSystem>();
    WorldBoundsSystem* world_bounds_system = m_system_context->GetSystem<WorldBoundsSystem>();
    RegionSystem* region_system = m_system_context->GetSystem<RegionSystem>();

    m_leveldata = ReadWorldComponentObjects(m_world_file, entity_system, nullptr);
    const game::LevelMetadata& metadata = m_leveldata.metadata;

    camera->SetPosition(metadata.camera_position);
    camera->SetViewportSize(metadata.camera_size);
    renderer->SetClearColor(metadata.background_color);
    renderer->SetAmbientShade(metadata.ambient_shade);

    SoundSystem* sound_system = m_system_context->GetSystem<SoundSystem>();
    sound_system->PlayBackgroundMusic(metadata.background_music, SoundTransition::CrossFade);

    // Nav mesh
    SetupNavmesh(m_navmesh, metadata, physics_system->GetSpace());
    game::g_navmesh = &m_navmesh;

    m_debug_input = std::make_unique<ImGuiInputHandler>(*m_event_handler);

    if(!metadata.background_texture.empty())
    {
        mono::StaticBackground* background = new mono::StaticBackground(
            metadata.background_size, metadata.background_texture.c_str(), mono::TextureModeFlags::REPEAT);
        AddDrawable(background, LayerId::BACKGROUND);
    }
    
    AddDrawable(new mono::RoadBatchDrawer(road_system, path_system, transform_system), LayerId::BACKGROUND);
    AddDrawable(new mono::SpriteBatchDrawer(transform_system, sprite_system), LayerId::GAMEOBJECTS);
    AddDrawable(new mono::TextBatchDrawer(text_system, transform_system), LayerId::GAMEOBJECTS);
    AddDrawable(new mono::ParticleSystemDrawer(particle_system, transform_system, mono::ParticleDrawLayer::PRE_GAMEOBJECTS), LayerId::PRE_GAMEOBJECTS);
    AddDrawable(new mono::ParticleSystemDrawer(particle_system, transform_system, mono::ParticleDrawLayer::POST_GAMEOBJECTS), LayerId::POST_GAMEOBJECTS);
    AddDrawable(new mono::LightSystemDrawer(light_system, transform_system), LayerId::GAMEOBJECTS);
    AddDrawable(new InteractionSystemDrawer(interaction_system, sprite_system, transform_system, entity_system), LayerId::UI);
    AddDrawable(new HealthbarDrawer(damage_system, transform_system, entity_system), LayerId::GAMEOBJECTS_UI);
    AddDrawable(new PlayerAuxiliaryDrawer(transform_system), LayerId::GAMEOBJECTS_UI);
    AddDrawable(new DialogSystemDrawer(dialog_system, transform_system), LayerId::UI);
    AddDrawable(new SpawnSystemDrawer(spawn_system, transform_system, particle_system, entity_system), LayerId::UI);
    AddDrawable(new WorldBoundsDrawer(transform_system, world_bounds_system), LayerId::FOG);

    m_region_ui = new RegionDrawer(region_system);
    AddUpdatableDrawable(m_region_ui, LayerId::UI);

    // Debug
    AddDrawable(new PhysicsStatsElement(physics_system), LayerId::UI);
    AddDrawable(new ParticleStatusDrawer(particle_system), LayerId::UI);
    AddDrawable(new NavmeshVisualizer(m_navmesh, *m_event_handler), LayerId::UI);
    AddDrawable(new mono::TransformSystemDrawer(g_draw_transformsystem, transform_system), LayerId::UI);
    AddDrawable(new mono::PhysicsDebugDrawer(
        g_draw_physics, g_interact_physics, g_body_introspection, g_draw_physics_subcomponents, physics_system, m_event_handler), LayerId::UI);
    AddDrawable(new TriggerDebugDrawer(g_draw_triggers, trigger_system, transform_system), LayerId::UI);
    AddDrawable(new DebugUpdater(m_system_context, m_event_handler, renderer), LayerId::UI);
    AddDrawable(new GameDebugDrawer(), LayerId::UI_DEBUG);

    m_game_mode = CreateGameMode();
    m_game_mode->Begin(this, renderer, m_system_context, m_event_handler, metadata);
    AddUpdatable(m_game_mode.get());
}

int GameZone::OnUnload()
{
    RemoveUpdatable(m_game_mode.get());
    const int game_mode_result = m_game_mode->End(this);
    m_game_mode = nullptr;

    game::g_navmesh = nullptr;

    RemoveUpdatableDrawable(m_region_ui);
    delete m_region_ui;
    m_region_ui = nullptr;

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
