
#include "GameZone.h"

#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Sprite/SpriteBatchDrawer.h"
#include "Rendering/Text/TextSystem.h"
#include "Rendering/Text/TextBatchDrawer.h"
#include "Rendering/Objects/StaticBackground.h"

#include "Physics/PhysicsSystem.h"
#include "Physics/PhysicsDebugDrawer.h"

#include "Particle/ParticleSystem.h"
#include "Particle/ParticleSystemDrawer.h"

#include "SystemContext.h"
#include "EntitySystem/EntitySystem.h"
#include "TransformSystem/TransformSystem.h"
#include "TransformSystem/TransformSystemDrawer.h"

#include "Player/PlayerInfo.h"
#include "Hud/HealthbarDrawer.h"
#include "Hud/Debug/PhysicsStatsElement.h"
#include "Hud/Debug/ConsoleDrawer.h"
#include "Hud/Debug/ParticleStatusDrawer.h"

#include "Navigation/NavMeshVisualizer.h"

#include "EntitySystem/IEntityManager.h"

#include "RenderLayers.h"
#include "DamageSystem.h"
#include "WorldFile.h"
#include "Camera/ICamera.h"
#include "GameDebug.h"
#include "GameDebugDrawer.h"

#include "InteractionSystem/InteractionSystem.h"
#include "InteractionSystem/InteractionSystemDrawer.h"

#include "TriggerSystem/TriggerSystem.h"
#include "TriggerSystem/TriggerDebugDrawer.h"

#include "ImGuiImpl/ImGuiInputHandler.h"

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
    game::g_navmesh = &m_navmesh;

    mono::EntitySystem* entity_system = m_system_context->GetSystem<mono::EntitySystem>();
    mono::TransformSystem* transform_system = m_system_context->GetSystem<mono::TransformSystem>();
    mono::PhysicsSystem* physics_system = m_system_context->GetSystem<mono::PhysicsSystem>();
    mono::SpriteSystem* sprite_system = m_system_context->GetSystem<mono::SpriteSystem>();
    mono::TextSystem* text_system = m_system_context->GetSystem<mono::TextSystem>();
    mono::ParticleSystem* particle_system = m_system_context->GetSystem<mono::ParticleSystem>();
    DamageSystem* damage_system = m_system_context->GetSystem<DamageSystem>();
    TriggerSystem* trigger_system = m_system_context->GetSystem<TriggerSystem>();
    InteractionSystem* interaction_system = m_system_context->GetSystem<InteractionSystem>();

    m_leveldata = shared::ReadWorldComponentObjects(m_world_file, entity_system, nullptr);
    camera->SetPosition(m_leveldata.metadata.camera_position);
    camera->SetViewportSize(m_leveldata.metadata.camera_size);

    m_debug_input = std::make_unique<ImGuiInputHandler>(*m_event_handler);

    if(!m_leveldata.metadata.background_texture.empty())
        AddDrawable(new mono::StaticBackground(m_leveldata.metadata.background_texture.c_str()), LayerId::BACKGROUND);
    
    AddDrawable(new mono::SpriteBatchDrawer(transform_system, sprite_system), LayerId::GAMEOBJECTS);
    AddDrawable(new mono::TextBatchDrawer(text_system, transform_system), LayerId::GAMEOBJECTS);
    AddDrawable(new mono::ParticleSystemDrawer(particle_system), LayerId::PARTICLES);
    AddDrawable(new InteractionSystemDrawer(interaction_system, sprite_system, transform_system), LayerId::UI);
    AddDrawable(new HealthbarDrawer(damage_system, transform_system, entity_system), LayerId::UI);

    // Debug
    AddDrawable(new GameDebugDrawer(), LayerId::GAMEOBJECTS_DEBUG);
    AddDrawable(new PhysicsStatsElement(physics_system), LayerId::UI);
    AddDrawable(new ParticleStatusDrawer(particle_system), LayerId::UI);
    AddDrawable(new NavmeshVisualizer(m_navmesh, *m_event_handler), LayerId::UI);
    AddDrawable(new mono::TransformSystemDrawer(g_draw_transformsystem, transform_system), LayerId::UI);
    AddDrawable(new mono::PhysicsDebugDrawer(g_draw_physics, g_draw_physics_subcomponents, physics_system, m_event_handler), LayerId::UI);
    AddDrawable(new TriggerDebugDrawer(g_draw_triggers, trigger_system, transform_system), LayerId::UI);
    AddDrawable(new DebugUpdater(trigger_system, m_event_handler), LayerId::UI);
}

int GameZone::OnUnload()
{
    mono::EntitySystem* entity_system = m_system_context->GetSystem<mono::EntitySystem>();
    entity_system->ReleaseAllEntities();

    return 0;
}
