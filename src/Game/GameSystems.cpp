
#include "GameSystems.h"

#include "SystemContext.h"
#include "EventHandler/EventHandler.h"

#include "EntitySystem/EntitySystem.h"
#include "Input/InputSystem.h"
#include "Particle/ParticleSystem.h"
#include "Paths/PathSystem.h"
#include "Physics/PhysicsSystem.h"
#include "Rendering/RenderSystem.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Text/TextSystem.h"
#include "Rendering/Lights/LightSystem.h"
#include "TransformSystem/TransformSystem.h"

#include "DamageSystem/DamageSystem.h"
#include "DialogSystem/DialogSystem.h"
#include "Entity/AnimationSystem.h"
#include "Entity/EntityLogicSystem.h"
#include "GameCamera/CameraSystem.h"
#include "InteractionSystem/InteractionSystem.h"
#include "Navigation/NavigationSystem.h"
#include "Pickups/PickupSystem.h"
#include "Player/PlayerDaemonSystem.h"
#include "TriggerSystem/TriggerSystem.h"
#include "SpawnSystem/SpawnSystem.h"
#include "RoadSystem/RoadSystem.h"
#include "Sound/SoundSystem.h"
#include "UI/UISystem.h"
#include "Weapons/WeaponSystem.h"
#include "World/RegionSystem.h"
#include "World/TeleportSystem.h"
#include "World/WorldBoundsSystem.h"
#include "World/WorldEntityTrackingSystem.h"

#include "Network/ServerManager.h"
#include "Network/ClientManager.h"

#include "Entity/Component.h"


void game::CreateGameSystems(
    uint32_t max_entities,
    mono::SystemContext& system_context,
    mono::EventHandler& event_handler,
    mono::ICamera& camera,
    const mono::RenderInitParams& render_params,
    const game::Config& game_config)
{
    mono::InputSystem* input_system = system_context.CreateSystem<mono::InputSystem>(&event_handler);
    mono::RenderSystem* render_system = system_context.CreateSystem<mono::RenderSystem>(max_entities, render_params);
    mono::EntitySystem* entity_system = system_context.CreateSystem<mono::EntitySystem>(
        max_entities, &system_context, component::ComponentNameFromHash, AttributeNameFromHash);
    mono::TransformSystem* transform_system = system_context.CreateSystem<mono::TransformSystem>(max_entities);
    mono::ParticleSystem* particle_system =
        system_context.CreateSystem<mono::ParticleSystem>(max_entities, 100, transform_system);

    mono::PhysicsSystemInitParams physics_system_params;
    physics_system_params.n_bodies = max_entities;
    physics_system_params.n_circle_shapes = max_entities;
    physics_system_params.n_segment_shapes = max_entities;
    physics_system_params.n_polygon_shapes = max_entities;

    mono::PhysicsSystem* physics_system = system_context.CreateSystem<mono::PhysicsSystem>(physics_system_params, transform_system);
    mono::SpriteSystem* sprite_system = system_context.CreateSystem<mono::SpriteSystem>(max_entities, transform_system);
    system_context.CreateSystem<mono::TextSystem>(max_entities, transform_system);
    system_context.CreateSystem<mono::PathSystem>(max_entities, transform_system);
    system_context.CreateSystem<mono::RoadSystem>(max_entities);
    system_context.CreateSystem<mono::LightSystem>(max_entities);

    game::DamageSystem* damage_system =
        system_context.CreateSystem<game::DamageSystem>(max_entities, transform_system, sprite_system, entity_system);
    game::TriggerSystem* trigger_system =
        system_context.CreateSystem<game::TriggerSystem>(max_entities, damage_system, physics_system, entity_system);
    system_context.CreateSystem<game::EntityLogicSystem>(max_entities, &system_context, &event_handler);
    system_context.CreateSystem<game::SpawnSystem>(max_entities, trigger_system, entity_system, transform_system);
    system_context.CreateSystem<game::PickupSystem>(
        max_entities, damage_system, transform_system, particle_system, physics_system, entity_system);
    system_context.CreateSystem<game::AnimationSystem>(max_entities, trigger_system, transform_system, sprite_system);
    game::CameraSystem* camera_system =
        system_context.CreateSystem<game::CameraSystem>(max_entities, &camera, transform_system, &event_handler, trigger_system);
    system_context.CreateSystem<game::InteractionSystem>(max_entities, transform_system, trigger_system);
    system_context.CreateSystem<game::DialogSystem>(max_entities);
    system_context.CreateSystem<game::SoundSystem>(max_entities, trigger_system);
    system_context.CreateSystem<game::RegionSystem>(physics_system);
    system_context.CreateSystem<game::WorldBoundsSystem>(transform_system);
    system_context.CreateSystem<game::WeaponSystem>(
        transform_system, sprite_system, physics_system, damage_system, camera_system, entity_system, &system_context);
    system_context.CreateSystem<game::UISystem>(input_system, transform_system, camera_system, trigger_system);
    system_context.CreateSystem<game::NavigationSystem>();
    system_context.CreateSystem<game::TeleportSystem>(camera_system, trigger_system, render_system, transform_system);
    system_context.CreateSystem<game::WorldEntityTrackingSystem>();

    game::ServerManager* server_manager = system_context.CreateSystem<game::ServerManager>(&event_handler, &game_config);
    system_context.CreateSystem<game::ClientManager>(&event_handler, &game_config);

    system_context.CreateSystem<game::PlayerDaemonSystem>(server_manager, entity_system, &system_context, &event_handler);
}
