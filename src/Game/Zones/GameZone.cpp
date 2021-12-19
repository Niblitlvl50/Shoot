
#include "GameZone.h"

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
#include "Hud/PlayerReloadDrawer.h"
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
#include "ConditionSystem/ConditionSystem.h"

#include "ImGuiImpl/ImGuiInputHandler.h"

#include "Util/Algorithm.h"
#include "CollisionConfiguration.h"

#include "GameMode/IGameMode.h"

namespace
{
    class DefaultGameMode : public game::IGameMode
    {
    public:

        void Begin(
            mono::IZone* zone,
            mono::IRenderer* renderer,
            mono::SystemContext* system_context,
            mono::EventHandler* event_handler,
            const math::Vector& player_spawn) override { }
        int End(mono::IZone* zone) override { return 0; }
        void Update(const mono::UpdateContext& update_context) override {}
    };

    void SetupNavmesh(game::NavmeshContext& navmesh_context, const shared::LevelMetadata& metadata, mono::PhysicsSpace* space)
    {
        navmesh_context.points = game::GenerateMeshPoints(metadata.navmesh_start, metadata.navmesh_end, metadata.navmesh_density);

        const auto remove_on_collision = [space](const math::Vector& point) {
            const mono::QueryResult query_result = space->QueryNearest(point, 0.0f, shared::CollisionCategory::STATIC);
            return query_result.body != nullptr;
        };
        mono::remove_if(navmesh_context.points, remove_on_collision);

        const auto filter_connection_func = [space](const math::Vector& first, const math::Vector& second){
            const mono::QueryResult query_result = space->QueryFirst(first, second, shared::CollisionCategory::STATIC);
            return query_result.body != nullptr;
        };

        navmesh_context.nodes = game::GenerateMeshNodes(navmesh_context.points, metadata.navmesh_density * 1.5f, filter_connection_func);
    }
}

using namespace game;

GameZone::GameZone(const ZoneCreationContext& context, const char* world_file)
    : m_system_context(context.system_context)
    , m_event_handler(context.event_handler)
    , m_world_file(world_file)
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
    ConditionSystem* condition_system = m_system_context->GetSystem<ConditionSystem>();

    m_leveldata = shared::ReadWorldComponentObjects(m_world_file, entity_system, nullptr);
    camera->SetPosition(m_leveldata.metadata.camera_position);
    camera->SetViewportSize(m_leveldata.metadata.camera_size);
    renderer->SetClearColor(m_leveldata.metadata.background_color);
    renderer->SetAmbientShade(m_leveldata.metadata.ambient_shade);

    for(const std::string& condition : m_leveldata.metadata.conditions)
        condition_system->RegisterCondition(hash::Hash(condition.c_str()), false);

    // Nav mesh
    SetupNavmesh(m_navmesh, m_leveldata.metadata, physics_system->GetSpace());
    game::g_navmesh = &m_navmesh;

    m_debug_input = std::make_unique<ImGuiInputHandler>(*m_event_handler);

    if(!m_leveldata.metadata.background_texture.empty())
    {
        AddDrawable(
            new mono::StaticBackground(
                m_leveldata.metadata.background_size,
                m_leveldata.metadata.background_texture.c_str(),
                mono::TextureModeFlags::REPEAT),
            LayerId::BACKGROUND);
    }
    
    AddDrawable(new mono::RoadBatchDrawer(road_system, path_system, transform_system), LayerId::BACKGROUND);
    AddDrawable(new mono::SpriteBatchDrawer(transform_system, sprite_system), LayerId::GAMEOBJECTS);
    AddDrawable(new mono::TextBatchDrawer(text_system, transform_system), LayerId::GAMEOBJECTS);
    AddDrawable(new mono::ParticleSystemDrawer(particle_system, transform_system), LayerId::PARTICLES);
    AddDrawable(new mono::LightSystemDrawer(light_system, transform_system), LayerId::GAMEOBJECTS);
    AddDrawable(new InteractionSystemDrawer(interaction_system, sprite_system, transform_system, entity_system), LayerId::UI);
    AddDrawable(new HealthbarDrawer(damage_system, transform_system, entity_system), LayerId::UI);
    AddDrawable(new PlayerReloadDrawer(transform_system), LayerId::UI);
    AddDrawable(new DialogSystemDrawer(dialog_system), LayerId::UI);

    // Debug
    AddDrawable(new GameDebugDrawer(), LayerId::GAMEOBJECTS_DEBUG);
    AddDrawable(new PhysicsStatsElement(physics_system), LayerId::UI);
    AddDrawable(new ParticleStatusDrawer(particle_system), LayerId::UI);
    AddDrawable(new NavmeshVisualizer(m_navmesh, *m_event_handler), LayerId::UI);
    AddDrawable(new mono::TransformSystemDrawer(g_draw_transformsystem, transform_system), LayerId::UI);
    AddDrawable(new mono::PhysicsDebugDrawer(g_draw_physics, g_interact_physics, g_draw_physics_subcomponents, physics_system, m_event_handler), LayerId::UI);
    AddDrawable(new TriggerDebugDrawer(g_draw_triggers, trigger_system, transform_system), LayerId::UI);
    AddDrawable(new SpawnSystemDrawer(spawn_system, transform_system, particle_system, entity_system), LayerId::UI);
    AddDrawable(new DebugUpdater(trigger_system, transform_system, entity_system, m_event_handler), LayerId::UI);

    m_game_mode = CreateGameMode();
    m_game_mode->Begin(this, renderer, m_system_context, m_event_handler, m_leveldata.metadata.player_spawn_point);

    AddUpdatable(m_game_mode.get());
}

int GameZone::OnUnload()
{
    RemoveUpdatable(m_game_mode.get());
    const int game_mode_result = m_game_mode->End(this);
    m_game_mode = nullptr;

    game::g_navmesh = nullptr;

    ZoneBase::OnUnload();

    mono::EntitySystem* entity_system = m_system_context->GetSystem<mono::EntitySystem>();
    entity_system->PopEntityStackRecord();

    ConditionSystem* condition_system = m_system_context->GetSystem<ConditionSystem>();
    condition_system->ClearAllConditions();

    return game_mode_result;
}

std::unique_ptr<IGameMode> GameZone::CreateGameMode()
{
    return std::make_unique<DefaultGameMode>();
}
