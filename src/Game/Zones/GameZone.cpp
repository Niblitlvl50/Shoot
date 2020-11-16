
#include "GameZone.h"

#include "Audio/AudioDebugDrawer.h"

#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Sprite/SpriteBatchDrawer.h"
#include "Rendering/Text/TextSystem.h"
#include "Rendering/Text/TextBatchDrawer.h"

#include "Physics/PhysicsSystem.h"
#include "Physics/PhysicsDebugDrawer.h"

#include "Particle/ParticleSystem.h"
#include "Particle/ParticleSystemDrawer.h"

#include "SystemContext.h"
#include "EntitySystem/EntitySystem.h"
#include "TransformSystem/TransformSystem.h"
#include "TransformSystem/TransformSystemDrawer.h"

#include "AIKnowledge.h"
#include "Hud/Overlay.h"
#include "Hud/Healthbar.h"
#include "Hud/Debug/FPSElement.h"
#include "Hud/Debug/PhysicsStatsElement.h"
#include "Hud/Debug/ConsoleDrawer.h"
#include "Hud/Debug/NetworkStatusDrawer.h"
#include "Hud/Debug/ClientViewportVisualizer.h"
#include "Hud/Debug/ParticleStatusDrawer.h"

#include "UpdateTasks/ListenerPositionUpdater.h"
#include "Navigation/NavMeshVisualizer.h"

#include "EntitySystem/IEntityManager.h"
#include "Factories.h"

#include "RenderLayers.h"
#include "DamageSystem.h"
#include "World/StaticBackground.h"
#include "WorldFile.h"
#include "Camera/ICamera.h"
#include "GameDebug.h"
#include "GameDebugDrawer.h"

#include "TriggerSystem.h"
#include "TriggerDebugDrawer.h"

#include "ImGuiImpl/ImGuiInputHandler.h"

using namespace game;

GameZone::GameZone(const ZoneCreationContext& context, const char* world_file)
    : m_system_context(context.system_context)
    , m_event_handler(context.event_handler)
    , m_world_file(world_file)
{ }

GameZone::~GameZone()
{ }

void GameZone::OnLoad(mono::ICamera* camera)
{
    game::g_navmesh = &m_navmesh;

    const shared::LevelData leveldata = shared::ReadWorldComponentObjects(m_world_file, g_entity_manager, nullptr);
    m_loaded_entities = leveldata.loaded_entities;
    m_player_spawn_point = leveldata.metadata.player_spawn_point;
    camera->SetPosition(leveldata.metadata.camera_position);
    camera->SetViewportSize(leveldata.metadata.camera_size);

    m_debug_input = std::make_unique<ImGuiInputHandler>(*m_event_handler);

    mono::EntitySystem* entity_system = m_system_context->GetSystem<mono::EntitySystem>();
    mono::TransformSystem* transform_system = m_system_context->GetSystem<mono::TransformSystem>();
    mono::PhysicsSystem* physics_system = m_system_context->GetSystem<mono::PhysicsSystem>();
    mono::SpriteSystem* sprite_system = m_system_context->GetSystem<mono::SpriteSystem>();
    mono::TextSystem* text_system = m_system_context->GetSystem<mono::TextSystem>();
    mono::ParticleSystem* particle_system = m_system_context->GetSystem<mono::ParticleSystem>();
    DamageSystem* damage_system = m_system_context->GetSystem<DamageSystem>();
    TriggerSystem* trigger_system = m_system_context->GetSystem<TriggerSystem>();

    if(!leveldata.metadata.background_texture.empty())
        AddDrawable(new StaticBackground(leveldata.metadata.background_texture.c_str()), LayerId::BACKGROUND);
    
    AddDrawable(new mono::SpriteBatchDrawer(transform_system, sprite_system), LayerId::GAMEOBJECTS);
    AddDrawable(new mono::TextBatchDrawer(text_system, transform_system), LayerId::GAMEOBJECTS);
    AddDrawable(new mono::ParticleSystemDrawer(particle_system), LayerId::PARTICLES);
    AddDrawable(new HealthbarDrawer(damage_system, transform_system, entity_system), LayerId::UI);

    // Debug
    UIOverlayDrawer* debug_hud_overlay = new UIOverlayDrawer();
    debug_hud_overlay->AddChild(new FPSElement(math::Vector(2.0f, 2.0f), mono::Color::BLACK));
    debug_hud_overlay->AddChild(new PhysicsStatsElement(physics_system, math::Vector(2.0f, 190.0f), mono::Color::BLACK));
    //debug_hud_overlay->AddChild(new NetworkStatusDrawer(math::Vector(2.0f, 190.0f), m_server_manager.get()));
    debug_hud_overlay->AddChild(new ParticleStatusDrawer(particle_system, math::Vector(2, 190)));
    AddEntity(debug_hud_overlay, LayerId::UI);

    AddUpdatable(new DebugUpdater(m_event_handler));
    AddDrawable(new GameDebugDrawer(), LayerId::GAMEOBJECTS_DEBUG);
    AddDrawable(new NavmeshVisualizer(m_navmesh, *m_event_handler), LayerId::UI);
    AddDrawable(new mono::TransformSystemDrawer(g_draw_transformsystem, transform_system), LayerId::UI);
    AddDrawable(new mono::PhysicsDebugDrawer(g_draw_physics, g_draw_physics_subcomponents, physics_system, m_event_handler), LayerId::UI);
    AddDrawable(new mono::AudioDebugDrawer(g_draw_audio), LayerId::UI);
    AddDrawable(new TriggerDebugDrawer(g_draw_triggers, trigger_system, transform_system), LayerId::UI);
}

int GameZone::OnUnload()
{
    for(uint32_t entity_id : m_loaded_entities)
        g_entity_manager->ReleaseEntity(entity_id);
    g_entity_manager->Sync();

    return 0;
}
