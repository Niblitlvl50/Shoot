
#include "SystemTestZone.h"

#include "Math/Matrix.h"

#include "EventHandler/EventHandler.h"
#include "Events/SpawnConstraintEvent.h"
#include "Events/GameEventFuncFwd.h"

#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Sprite/SpriteBatchDrawer.h"
#include "Rendering/Sprite/SpriteFactory.h"
#include "Rendering/Sprite/Sprite.h"

#include "Physics/PhysicsSystem.h"
#include "Physics/PhysicsDebugDrawer.h"
#include "Physics/IBody.h"

#include "SystemContext.h"
#include "TransformSystem.h"
#include "EntitySystem.h"
#include "Util/Random.h"

#include "AIKnowledge.h"

#include "Hud/WeaponStatusElement.h"
#include "Hud/Overlay.h"
#include "Hud/FPSElement.h"
#include "Hud/PhysicsStatsElement.h"
#include "Hud/PickupDrawer.h"
#include "Hud/WaveDrawer.h"
#include "Hud/ConsoleDrawer.h"
#include "Hud/Healthbar.h"

#include "Navigation/NavmeshFactory.h"
#include "Navigation/NavMeshVisualizer.h"

#include "UpdateTasks/ListenerPositionUpdater.h"
#include "UpdateTasks/CameraViewportReporter.h"
#include "UpdateTasks/PickupUpdater.h"

#include "Pickups/Ammo.h"

#include "Entity/IEntityManager.h"
#include "Factories.h"
#include "DamageSystem.h"

#include "RenderLayers.h"
#include "Player/PlayerDaemon.h"

#include "WorldFile.h"

using namespace game;

namespace
{
    class FloorColisions : public mono::ICollisionHandler
    {
    public:
        FloorColisions(mono::SystemContext* system_context)
            : m_sprite_system(system_context->GetSystem<mono::SpriteSystem>())
            , m_transform_system(system_context->GetSystem<mono::TransformSystem>())
            , m_physics_system(system_context->GetSystem<mono::PhysicsSystem>())
            , m_damage_system(system_context->GetSystem<game::DamageSystem>())
        { }

        void OnCollideWith(mono::IBody* body, const math::Vector& collision_point, uint32_t categories) override
        {
            const uint32_t entity_id = m_physics_system->GetIdFromBody(body);
            m_damage_system->ApplyDamage(entity_id, 122);

            mono::Entity explosion_entity = entity_manager->CreateEntity("res/entities/tiny_explosion.entity");

            math::Matrix& transform = m_transform_system->GetTransform(explosion_entity.id);
            math::Identity(transform);
            math::Translate(transform, collision_point);

            const auto destroy_func = [explosion_entity] {
                entity_manager->ReleaseEntity(explosion_entity.id);
            };

            mono::ISprite* sprite = m_sprite_system->GetSprite(explosion_entity.id);
            sprite->SetAnimation(0, destroy_func);
        }

        mono::SpriteSystem* m_sprite_system;
        mono::TransformSystem* m_transform_system;
        const mono::PhysicsSystem* m_physics_system;
        game::DamageSystem* m_damage_system;
    };

    class SyncPoint : public mono::IUpdatable
    {
        void doUpdate(uint32_t delta_ms)
        {
            entity_manager->Sync();
        }
    };
}

SystemTestZone::SystemTestZone(const ZoneCreationContext& context)
    : m_system_context(context.system_context)
    , m_event_handler(context.event_handler)
{
        using namespace std::placeholders;

    const game::SpawnConstraintFunc& spawn_constraint_func = std::bind(&SystemTestZone::SpawnConstraint, this, _1);
    const game::DespawnConstraintFunc& despawn_constraint_func = std::bind(&SystemTestZone::DespawnConstraint, this, _1);

    m_spawn_constraint_token = m_event_handler->AddListener(spawn_constraint_func);
    m_despawn_constraint_token = m_event_handler->AddListener(despawn_constraint_func);
}

SystemTestZone::~SystemTestZone()
{
    m_event_handler->RemoveListener(m_spawn_constraint_token);
    m_event_handler->RemoveListener(m_despawn_constraint_token);
}

void SystemTestZone::OnLoad(mono::ICameraPtr& camera)
{
    // Syncing should be done first in the frame.
    AddUpdatable(std::make_shared<SyncPoint>());
    AddUpdatable(std::make_shared<ListenerPositionUpdater>());
    AddUpdatable(std::make_shared<CameraViewportReporter>(camera));
    AddUpdatable(std::make_shared<PickupUpdater>(m_pickups, *m_event_handler));

    AddDrawable(std::make_shared<PickupDrawer>(m_pickups), LayerId::GAMEOBJECTS);
    AddDrawable(std::make_shared<WaveDrawer>(*m_event_handler), LayerId::UI);

    m_console_drawer = std::make_shared<ConsoleDrawer>();
    AddDrawable(m_console_drawer, LayerId::UI);

    mono::PhysicsSystem* physics_system = m_system_context->GetSystem<mono::PhysicsSystem>();

    for(int index = 0; index < 250; ++index)
    {
        mono::Entity new_entity = entity_manager->CreateEntity("res/entities/pink_blolb.entity");
        const math::Vector position = math::Vector(mono::Random(0.0f, 22.0f), mono::Random(0.0f, 24.0f));
        mono::IBody* body = physics_system->GetBody(new_entity.id);
        body->SetPosition(position);

        m_loaded_entities.push_back(new_entity.id);
    }

    //mono::IBody* floor_body = physics_system->GetBody(floor_entity.id);
    //floor_body->SetCollisionHandler(new FloorColisions(m_system_context));

    const std::vector<uint32_t>& loaded_entities = world::ReadWorldComponentObjects("res/world.components", entity_manager);
    m_loaded_entities.insert(m_loaded_entities.end(), loaded_entities.begin(), loaded_entities.end());

    {
        // Nav mesh
        std::vector<ExcludeZone> exclude_zones;
        m_navmesh.points = game::GenerateMeshPoints(math::Vector(-100, -50), 150, 100, 3, exclude_zones);
        m_navmesh.nodes = game::GenerateMeshNodes(m_navmesh.points, 5, exclude_zones);
        game::g_navmesh = &m_navmesh;
        
        //AddDrawable(std::make_shared<NavmeshVisualizer>(m_navmesh, *m_event_handler), UI);
    }

    std::vector<math::Vector> player_points;
    m_player_daemon = std::make_unique<PlayerDaemon>(camera, player_points, m_system_context, *m_event_handler);

    auto sprite_drawer = std::make_shared<mono::SpriteBatchDrawer>(m_system_context);
    AddDrawable(sprite_drawer, GAMEOBJECTS);

    auto physics_debug_drawer = std::make_shared<mono::PhysicsDebugDrawer>(physics_system);
    AddDrawable(physics_debug_drawer, UI);

    mono::TransformSystem* transform_system = m_system_context->GetSystem<mono::TransformSystem>();
    DamageSystem* damage_system = m_system_context->GetSystem<DamageSystem>();

    AddDrawable(std::make_shared<HealthbarDrawer>(damage_system, transform_system), LayerId::UI);

    auto hud_overlay = std::make_shared<UIOverlayDrawer>();
    hud_overlay->AddChild(std::make_shared<WeaponStatusElement>(g_player_one, math::Vector(10.0f, 10.0f), math::Vector(-50.0f, 10.0f)));
    hud_overlay->AddChild(std::make_shared<WeaponStatusElement>(g_player_two, math::Vector(277.0f, 10.0f), math::Vector(320.0f, 10.0f)));
    hud_overlay->AddChild(std::make_shared<FPSElement>(math::Vector(2.0f, 2.0f), mono::Color::BLACK));
    hud_overlay->AddChild(std::make_shared<PhysicsStatsElement>(physics_system, math::Vector(2.0f, 190.0f), mono::Color::BLACK));
    AddEntity(hud_overlay, UI);
}

int SystemTestZone::OnUnload()
{
    for(uint32_t entity_id : m_loaded_entities)
        entity_manager->ReleaseEntity(entity_id);

    entity_manager->Sync();

    return 0;
}

bool SystemTestZone::SpawnConstraint(const game::SpawnConstraintEvent& event)
{
    return true;
}

bool SystemTestZone::DespawnConstraint(const game::DespawnConstraintEvent& event)
{
    return true;
}
