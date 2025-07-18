
#include "GameComponentFuncs.h"

#include "SystemContext.h"
#include "TransformSystem/TransformSystem.h"

#include "DamageSystem/DamageSystem.h"
#include "Sound/SoundSystem.h"
#include "TriggerSystem/TriggerSystem.h"
#include "Pickups/PickupSystem.h"
#include "SpawnSystem/SpawnSystem.h"
#include "EntitySystem/IEntityManager.h"
#include "Entity/EntityLogicSystem.h"
#include "Entity/AnimationSystem.h"
#include "Entity/TargetSystem.h"
#include "Entity/EntityLifetimeTriggerSystem.h"
#include "GameCamera/CameraSystem.h"
#include "GamePhysics/GamePhysicsSystem.h"
#include "InteractionSystem/InteractionSystem.h"
#include "DialogSystem/DialogSystem.h"
#include "Mission/MissionSystem.h"
#include "Weapons/WeaponSystem.h"
#include "World/TeleportSystem.h"
#include "World/WorldEntityTrackingSystem.h"

#include "Physics/PhysicsSystem.h"
#include "Pickups/PickupSystem.h"

#include "Component.h"
#include "CollisionConfiguration.h"

#include "System/Hash.h"
#include "System/System.h"
#include "Util/Random.h"
#include "Math/EasingFunctions.h"

namespace
{
    bool CreatePhysics(mono::Entity* entity, mono::SystemContext* context)
    {
        mono::BodyComponent body_params;
        body_params.mass = 1.0f;
        body_params.inertia = 1.0f;
        body_params.type = mono::BodyType::DYNAMIC;

        mono::PhysicsSystem* physics_system = context->GetSystem<mono::PhysicsSystem>();
        physics_system->AllocateBody(entity->id, body_params);
        return true;
    }

    bool ReleasePhysics(mono::Entity* entity, mono::SystemContext* context)
    {
        mono::PhysicsSystem* physics_system = context->GetSystem<mono::PhysicsSystem>();
        physics_system->ReleaseBody(entity->id);
        return true;
    }

    bool UpdatePhysics(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        mono::BodyComponent body_args;

        FindAttribute(BODY_TYPE_ATTRIBUTE, properties, (int&)body_args.type, FallbackMode::SET_DEFAULT);
        FindAttribute(MASS_ATTRIBUTE, properties, body_args.mass, FallbackMode::SET_DEFAULT);
        FindAttribute(INERTIA_ATTRIBUTE, properties, body_args.inertia, FallbackMode::SET_DEFAULT);

        bool prevent_rotation = false;
        FindAttribute(PREVENT_ROTATION_ATTRIBUTE, properties, prevent_rotation, FallbackMode::SET_DEFAULT);
        
        bool use_custom_damping = false;
        float custom_damping = 0.0f;
        FindAttribute(USE_CUSTOM_DAMPING, properties, use_custom_damping, FallbackMode::SET_DEFAULT);
        FindAttribute(DAMPING_ATTRIBUTE, properties, custom_damping, FallbackMode::SET_DEFAULT);

        uint32_t material;
        FindAttribute(PHYSICS_MATERIAL_ATTRIBUTE, properties, material, FallbackMode::SET_DEFAULT);

        mono::PhysicsSystem* physics_system = context->GetSystem<mono::PhysicsSystem>();
        mono::IBody* body = physics_system->GetBody(entity->id);
        body->SetMass(body_args.mass);
        body->SetPreventRotation(prevent_rotation);
        body->SetMoment(body_args.inertia);
        if(use_custom_damping)
            body->SetCustomDamping(custom_damping);
        body->SetType(body_args.type);
        body->SetMaterial(material);

        mono::TransformSystem* transform_system = context->GetSystem<mono::TransformSystem>();
        const math::Matrix& transform = transform_system->GetTransform(entity->id);
        body->SetPosition(math::GetPosition(transform));
        return true;
    }

    bool CreateShape(mono::Entity* entity, mono::SystemContext* context)
    {
        return true;
    }

    bool ReleaseShape(mono::Entity* entity, mono::SystemContext* context)
    {
        return true;
    }

    bool UpdateCircleShape(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        int faction = 0;
        mono::CircleComponent shape_params;

        FindAttribute(FACTION_ATTRIBUTE, properties, faction, FallbackMode::SET_DEFAULT);
        FindAttribute(RADIUS_ATTRIBUTE, properties, shape_params.radius, FallbackMode::SET_DEFAULT);
        FindAttribute(POSITION_ATTRIBUTE, properties, shape_params.offset, FallbackMode::SET_DEFAULT);
        FindAttribute(SENSOR_ATTRIBUTE, properties, shape_params.is_sensor, FallbackMode::SET_DEFAULT);

        const game::FactionPair& faction_pair = game::g_faction_lookup_table[faction];
        shape_params.category = faction_pair.category;
        shape_params.mask = faction_pair.mask;

        mono::PhysicsSystem* physics_system = context->GetSystem<mono::PhysicsSystem>();
        physics_system->AddShape(entity->id, shape_params);
        return true;
    }

    bool UpdateBoxShape(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        int faction;
        mono::BoxComponent shape_params;

        FindAttribute(FACTION_ATTRIBUTE, properties, faction, FallbackMode::SET_DEFAULT);
        FindAttribute(SIZE_ATTRIBUTE, properties, shape_params.size, FallbackMode::SET_DEFAULT);
        FindAttribute(POSITION_ATTRIBUTE, properties, shape_params.offset, FallbackMode::SET_DEFAULT);
        FindAttribute(SENSOR_ATTRIBUTE, properties, shape_params.is_sensor, FallbackMode::SET_DEFAULT);

        const game::FactionPair& faction_pair = game::g_faction_lookup_table[faction];
        shape_params.category = faction_pair.category;
        shape_params.mask = faction_pair.mask;

        mono::PhysicsSystem* physics_system = context->GetSystem<mono::PhysicsSystem>();
        physics_system->AddShape(entity->id, shape_params);
        return true;
    }

    bool UpdateSegmentShape(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        int faction;
        mono::SegmentComponent shape_params;

        FindAttribute(FACTION_ATTRIBUTE, properties, faction, FallbackMode::SET_DEFAULT);
        FindAttribute(RADIUS_ATTRIBUTE, properties, shape_params.radius, FallbackMode::SET_DEFAULT);
        FindAttribute(START_ATTRIBUTE, properties, shape_params.start, FallbackMode::SET_DEFAULT);
        FindAttribute(END_ATTRIBUTE, properties, shape_params.end, FallbackMode::SET_DEFAULT);
        FindAttribute(SENSOR_ATTRIBUTE, properties, shape_params.is_sensor, FallbackMode::SET_DEFAULT);

        const game::FactionPair& faction_pair = game::g_faction_lookup_table[faction];
        shape_params.category = faction_pair.category;
        shape_params.mask = faction_pair.mask;

        mono::PhysicsSystem* physics_system = context->GetSystem<mono::PhysicsSystem>();
        physics_system->AddShape(entity->id, shape_params);
        return true;
    }

    bool UpdatePolygonShape(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        int faction;
        mono::PolyComponent shape_params;

        FindAttribute(FACTION_ATTRIBUTE, properties, faction, FallbackMode::SET_DEFAULT);
        FindAttribute(POLYGON_ATTRIBUTE, properties, shape_params.vertices, FallbackMode::SET_DEFAULT);
        FindAttribute(SENSOR_ATTRIBUTE, properties, shape_params.is_sensor, FallbackMode::SET_DEFAULT);

        const game::FactionPair& faction_pair = game::g_faction_lookup_table[faction];
        shape_params.category = faction_pair.category;
        shape_params.mask = faction_pair.mask;

        mono::PhysicsSystem* physics_system = context->GetSystem<mono::PhysicsSystem>();
        physics_system->AddShape(entity->id, shape_params);
        return true;
    }

    bool CreateHealth(mono::Entity* entity, mono::SystemContext* context)
    {
        game::DamageSystem* damage_system = context->GetSystem<game::DamageSystem>();
        damage_system->CreateRecord(entity->id);
        return true;
    }

    bool ReleaseHealth(mono::Entity* entity, mono::SystemContext* context)
    {
        game::DamageSystem* damage_system = context->GetSystem<game::DamageSystem>();
        damage_system->ReleaseRecord(entity->id);
        return true;
    }

    bool UpdateHealth(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        int health;
        bool release_on_death;
        bool is_boss_health;

        FindAttribute(HEALTH_ATTRIBUTE, properties, health, FallbackMode::SET_DEFAULT);
        FindAttribute(RELEASE_ON_DEATH_ATTRIBUTE, properties, release_on_death, FallbackMode::SET_DEFAULT);
        FindAttribute(BOSS_HEALTH_ATTRIBUTE, properties, is_boss_health, FallbackMode::SET_DEFAULT);

        game::DamageSystem* damage_system = context->GetSystem<game::DamageSystem>();
        game::DamageRecord* damage_record = damage_system->GetDamageRecord(entity->id);
        damage_record->health = health;
        damage_record->full_health = health;
        damage_record->release_entity_on_death = release_on_death;
        damage_record->is_boss = is_boss_health;

        return true;
    }

    bool CreateShockwave(mono::Entity* entity, mono::SystemContext* context)
    {
        game::DamageSystem* damage_system = context->GetSystem<game::DamageSystem>();
        damage_system->CreateShockwaveComponent(entity->id);
        return true;
    }

    bool ReleaseShockwave(mono::Entity* entity, mono::SystemContext* context)
    {
        game::DamageSystem* damage_system = context->GetSystem<game::DamageSystem>();
        damage_system->ReleaseShockwaveComponent(entity->id);
        return true;
    }

    bool UpdateShockwave(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        uint32_t trigger = 0;
        float radius;
        math::Interval magnitude;
        int damage;

        mono::Event trigger_name;
        const bool found_enable = FindAttribute(TRIGGER_NAME_ATTRIBUTE, properties, trigger_name, FallbackMode::SET_DEFAULT);
        if(found_enable)
            trigger = hash::Hash(trigger_name.text.c_str());

        FindAttribute(RADIUS_ATTRIBUTE, properties, radius, FallbackMode::SET_DEFAULT);
        FindAttribute(MAGNITUDE_INTERVAL_ATTRIBUTE, properties, magnitude, FallbackMode::SET_DEFAULT);
        FindAttribute(HEALTH_ATTRIBUTE, properties, damage, FallbackMode::SET_DEFAULT);

        game::DamageSystem* damage_system = context->GetSystem<game::DamageSystem>();
        damage_system->UpdateShockwaveComponent(entity->id, trigger, radius, mono::Random(magnitude.min, magnitude.max), damage);

        return true;
    }

    bool CreateEntityLogic(mono::Entity* entity, mono::SystemContext* context)
    {
        game::EntityLogicSystem* logic_system = context->GetSystem<game::EntityLogicSystem>();
        (void)logic_system;
        return true;
    }

    bool ReleaseEntityLogic(mono::Entity* entity, mono::SystemContext* context)
    {
        game::EntityLogicSystem* logic_system = context->GetSystem<game::EntityLogicSystem>();
        logic_system->ReleaseLogic(entity->id);
        return true;
    }

    bool UpdateEntityLogic(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        int logic_type_value;
        const bool found_property = FindAttribute(ENTITY_BEHAVIOUR_ATTRIBUTE, properties, logic_type_value, FallbackMode::REQUIRE_ATTRIBUTE);
        if(!found_property)
            return false;

        game::EntityLogicSystem* logic_system = context->GetSystem<game::EntityLogicSystem>();
        game::IEntityLogic* entity_logic = logic_system->CreateLogic(game::EntityLogicType(logic_type_value), properties, entity->id);
        logic_system->AddLogic(entity->id, entity_logic);

        return true;
    }

    bool CreateSpawnPoint(mono::Entity* entity, mono::SystemContext* context)
    {
        game::SpawnSystem* spawn_system = context->GetSystem<game::SpawnSystem>();
        spawn_system->AllocateSpawnPoint(entity->id);
        return true;
    }

    bool ReleaseSpawnPoint(mono::Entity* entity, mono::SystemContext* context)
    {
        game::SpawnSystem* spawn_system = context->GetSystem<game::SpawnSystem>();
        spawn_system->ReleaseSpawnPoint(entity->id);
        return true;
    }

    bool UpdateSpawnPoint(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        game::SpawnSystem::SpawnPointComponent spawn_point;
        spawn_point.enable_trigger = 0;
        spawn_point.disable_trigger = 0;

        FindAttribute(SPAWN_SCORE_ATTRIBUTE, properties, spawn_point.spawn_score, FallbackMode::SET_DEFAULT);
        FindAttribute(SPAWN_LIMIT_ATTRIBUTE, properties, spawn_point.spawn_limit_total, FallbackMode::SET_DEFAULT);
        FindAttribute(SPAWN_LIMIT_CONCURRENT_ATTRIBUTE, properties, spawn_point.spawn_limit_concurrent, FallbackMode::SET_DEFAULT);
        FindAttribute(RADIUS_ATTRIBUTE, properties, spawn_point.radius, FallbackMode::SET_DEFAULT);
        FindAttribute(TIME_STAMP_ATTRIBUTE, properties, spawn_point.interval_ms, FallbackMode::SET_DEFAULT);

        mono::Event enable_trigger;
        const bool found_enable = FindAttribute(ENABLE_TRIGGER_ATTRIBUTE, properties, enable_trigger, FallbackMode::SET_DEFAULT);
        if(found_enable && !enable_trigger.text.empty())
            spawn_point.enable_trigger = hash::Hash(enable_trigger.text.c_str());

        mono::Event disable_trigger;
        const bool found_disable = FindAttribute(DISABLE_TRIGGER_ATTRIBUTE, properties, disable_trigger, FallbackMode::SET_DEFAULT);
        if(found_disable && !disable_trigger.text.empty())
            spawn_point.disable_trigger = hash::Hash(disable_trigger.text.c_str());

        FindAttribute(SPAWN_POINTS_ATTRIBUTE, properties, spawn_point.points, FallbackMode::SET_DEFAULT);

        game::SpawnSystem* spawn_system = context->GetSystem<game::SpawnSystem>();
        spawn_system->SetSpawnPointData(entity->id, spawn_point);
        return true;
    }

    bool CreateEntitySpawnPoint(mono::Entity* entity, mono::SystemContext* context)
    {
        game::SpawnSystem* spawn_system = context->GetSystem<game::SpawnSystem>();
        spawn_system->AllocateEntitySpawnPoint(entity->id);
        return true;
    }

    bool ReleaseEntitySpawnPoint(mono::Entity* entity, mono::SystemContext* context)
    {
        game::SpawnSystem* spawn_system = context->GetSystem<game::SpawnSystem>();
        spawn_system->ReleaseEntitySpawnPoint(entity->id);
        return true;
    }

    bool UpdateEntitySpawnPoint(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        std::string entity_file;
        float spawn_radius = 0.0f;
        uint32_t spawn_trigger = 0;

        const bool success = FindAttribute(ENTITY_FILE_ATTRIBUTE, properties, entity_file, FallbackMode::REQUIRE_ATTRIBUTE);
        if(!success)
        {
            System::Log("UpdateEntitySpawnPoint|Missing entity file parameters, unable to update component");
            return false;
        }

        FindAttribute(RADIUS_ATTRIBUTE, properties, spawn_radius, FallbackMode::SET_DEFAULT);

        mono::Event spawn_trigger_name;
        const bool found_enable = FindAttribute(TRIGGER_NAME_ATTRIBUTE, properties, spawn_trigger_name, FallbackMode::SET_DEFAULT);
        if(found_enable)
            spawn_trigger = hash::Hash(spawn_trigger_name.text.c_str());

        game::SpawnSystem* spawn_system = context->GetSystem<game::SpawnSystem>();
        spawn_system->SetEntitySpawnPointData(entity->id, entity_file, spawn_radius, spawn_trigger);
        return true;
    }

    bool CreateDespawnEntity(mono::Entity* entity, mono::SystemContext* context)
    {
        game::SpawnSystem* spawn_system = context->GetSystem<game::SpawnSystem>();
        spawn_system->AllocateDespawnTrigger(entity->id);
        return true;
    }
    
    bool ReleaseDespawnEntity(mono::Entity* entity, mono::SystemContext* context)
    {
        game::SpawnSystem* spawn_system = context->GetSystem<game::SpawnSystem>();
        spawn_system->ReleaseDespawnTrigger(entity->id);
        return true;
    }

    bool UpdateDespawnEntity(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        mono::Event despawn_trigger_name;
        const bool found_enable = FindAttribute(TRIGGER_NAME_ATTRIBUTE, properties, despawn_trigger_name, FallbackMode::SET_DEFAULT);
        if(!found_enable)
            return false;
    
        const uint32_t despawn_trigger = hash::Hash(despawn_trigger_name.text.c_str());
        game::SpawnSystem* spawn_system = context->GetSystem<game::SpawnSystem>();
        spawn_system->SetDespawnTriggerData(entity->id, despawn_trigger);
        return true;
    }

    bool CreateShapeTrigger(mono::Entity* entity, mono::SystemContext* context)
    {
        mono::TriggerSystem* trigger_system = context->GetSystem<mono::TriggerSystem>();
        trigger_system->AllocateShapeTrigger(entity->id);
        return true;
    }
    
    bool ReleaseShapeTrigger(mono::Entity* entity, mono::SystemContext* context)
    {
        mono::TriggerSystem* trigger_system = context->GetSystem<mono::TriggerSystem>();
        trigger_system->ReleaseShapeTrigger(entity->id);
        return true;
    }
    
    bool UpdateShapeTrigger(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        mono::Event enter_trigger_name;
        const bool found_trigger_name =
            FindAttribute(TRIGGER_NAME_ATTRIBUTE, properties, enter_trigger_name, FallbackMode::REQUIRE_ATTRIBUTE);

        mono::Event exit_trigger_name;
        const bool found_exit_trigger_name =
            FindAttribute(TRIGGER_NAME_EXIT_ATTRIBUTE, properties, exit_trigger_name, FallbackMode::REQUIRE_ATTRIBUTE);

        if(!found_trigger_name && !found_exit_trigger_name)
        {
            System::Log("GameComponentFunctions|Missing trigger name parameter, unable to update component");
            return false;
        }

        uint32_t faction;
        bool emit_once;
        FindAttribute(FACTION_PICKER_ATTRIBUTE, properties, faction, FallbackMode::SET_DEFAULT);
        FindAttribute(EMIT_ONCE_ATTRIBUTE, properties, emit_once, FallbackMode::SET_DEFAULT);

        const uint32_t enter_trigger_hash = hash::Hash(enter_trigger_name.text.c_str());
        const uint32_t exit_trigger_hash = hash::Hash(exit_trigger_name.text.c_str());

        mono::TriggerSystem* trigger_system = context->GetSystem<mono::TriggerSystem>();
        trigger_system->AddShapeTrigger(entity->id, enter_trigger_hash, exit_trigger_hash, faction, emit_once);

        hash::HashRegisterString(enter_trigger_name.text.c_str());
        hash::HashRegisterString(exit_trigger_name.text.c_str());

        return true;
    }

    bool CreateDestroyedTrigger(mono::Entity* entity, mono::SystemContext* context)
    {
        game::EntityLifetimeTriggerSystem* trigger_system = context->GetSystem<game::EntityLifetimeTriggerSystem>();
        trigger_system->AllocateDestroyedTrigger(entity->id);
        return true;
    }

    bool ReleaseDestroyedTrigger(mono::Entity* entity, mono::SystemContext* context)
    {
        game::EntityLifetimeTriggerSystem* trigger_system = context->GetSystem<game::EntityLifetimeTriggerSystem>();
        trigger_system->ReleaseDestroyedTrigger(entity->id);
        return true;
    }

    bool UpdateDestroyedTrigger(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        mono::Event trigger_name;
        const bool found_trigger_name =
            FindAttribute(TRIGGER_NAME_ATTRIBUTE, properties, trigger_name, FallbackMode::REQUIRE_ATTRIBUTE);

        if(!found_trigger_name)
        {
            System::Log("GameComponentFunctions|Missing trigger name parameter, unable to update component");
            return false;
        }

        int trigger_type;
        FindAttribute(DESTROYED_TRIGGER_TYPE_ATTRIBUTE, properties, trigger_type, FallbackMode::SET_DEFAULT);

        const uint32_t trigger_hash = hash::Hash(trigger_name.text.c_str());
        game::EntityLifetimeTriggerSystem* trigger_system = context->GetSystem<game::EntityLifetimeTriggerSystem>();
        trigger_system->AddDestroyedTrigger(entity->id, trigger_hash, game::DestroyedTriggerType(trigger_type));
        hash::HashRegisterString(trigger_name.text.c_str());

        return true;
    }

    bool CreateAreaTrigger(mono::Entity* entity, mono::SystemContext* context)
    {
        mono::TriggerSystem* trigger_system = context->GetSystem<mono::TriggerSystem>();
        trigger_system->AllocateAreaTrigger(entity->id);
        return true;
    }

    bool ReleaseAreaTrigger(mono::Entity* entity, mono::SystemContext* context)
    {
        mono::TriggerSystem* trigger_system = context->GetSystem<mono::TriggerSystem>();
        trigger_system->ReleaseAreaTrigger(entity->id);
        return true;
    }

    bool UpdateAreaTrigger(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        mono::Event trigger_name;
        const bool found_trigger_name =
            FindAttribute(TRIGGER_NAME_ATTRIBUTE, properties, trigger_name, FallbackMode::REQUIRE_ATTRIBUTE);
        if(!found_trigger_name)
            return false;

        math::Vector size;
        uint32_t faction;
        int operation;
        int n_entities;

        FindAttribute(SIZE_ATTRIBUTE, properties, size, FallbackMode::SET_DEFAULT);
        FindAttribute(FACTION_PICKER_ATTRIBUTE, properties, faction, FallbackMode::SET_DEFAULT);
        FindAttribute(LOGIC_OP_ATTRIBUTE, properties, operation, FallbackMode::SET_DEFAULT);
        FindAttribute(N_ENTITIES_ATTRIBUTE, properties, n_entities, FallbackMode::SET_DEFAULT);

        const math::Vector half_width_height = size / 2.0f;

        mono::TransformSystem* transform_system = context->GetSystem<mono::TransformSystem>();
        const math::Matrix& world_transform = transform_system->GetWorld(entity->id);
        const math::Quad world_bb = math::Transformed(world_transform, math::Quad(-half_width_height, half_width_height));

        const uint32_t trigger_hash = hash::Hash(trigger_name.text.c_str());

        mono::TriggerSystem* trigger_system = context->GetSystem<mono::TriggerSystem>();
        trigger_system->AddAreaEntityTrigger(
            entity->id, trigger_hash, world_bb, faction, mono::AreaTriggerOperation(operation), n_entities);
        hash::HashRegisterString(trigger_name.text.c_str());

        return true;
    }

    bool CreateTimeTrigger(mono::Entity* entity, mono::SystemContext* context)
    {
        mono::TriggerSystem* trigger_system = context->GetSystem<mono::TriggerSystem>();
        trigger_system->AllocateTimeTrigger(entity->id);
        return true;
    }

    bool ReleaseTimeTrigger(mono::Entity* entity, mono::SystemContext* context)
    {
        mono::TriggerSystem* trigger_system = context->GetSystem<mono::TriggerSystem>();
        trigger_system->ReleaseTimeTrigger(entity->id);
        return true;
    }

    bool UpdateTimeTrigger(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        mono::Event trigger_name;
        const bool found_trigger_name =
            FindAttribute(TRIGGER_NAME_ATTRIBUTE, properties, trigger_name, FallbackMode::REQUIRE_ATTRIBUTE);
        if(!found_trigger_name)
            return false;

        int timeout_ms;
        bool repeating;
        FindAttribute(TIME_STAMP_ATTRIBUTE, properties, timeout_ms, FallbackMode::SET_DEFAULT);
        FindAttribute(REPEATING_ATTRIBUTE, properties, repeating, FallbackMode::SET_DEFAULT);

        const uint32_t trigger_hash = hash::Hash(trigger_name.text.c_str());
        mono::TriggerSystem* trigger_system = context->GetSystem<mono::TriggerSystem>();
        trigger_system->AddTimeTrigger(entity->id, trigger_hash, timeout_ms, repeating);
        hash::HashRegisterString(trigger_name.text.c_str());

        return true;
    }

    bool CreateCounterTrigger(mono::Entity* entity, mono::SystemContext* context)
    {
        mono::TriggerSystem* trigger_system = context->GetSystem<mono::TriggerSystem>();
        trigger_system->AllocateCounterTrigger(entity->id);
        return true;
    }

    bool ReleaseCounterTrigger(mono::Entity* entity, mono::SystemContext* context)
    {
        mono::TriggerSystem* trigger_system = context->GetSystem<mono::TriggerSystem>();
        trigger_system->ReleaseCounterTrigger(entity->id);
        return true;
    }

    bool UpdateCounterTrigger(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        mono::Event trigger_name;
        mono::Event trigger_name_completed;
        const bool found_trigger_name =
            FindAttribute(TRIGGER_NAME_ATTRIBUTE, properties, trigger_name, FallbackMode::REQUIRE_ATTRIBUTE);
        const bool found_trigger_name_completed =
            FindAttribute(TRIGGER_NAME_COMPLETED_ATTRIBUTE, properties, trigger_name_completed, FallbackMode::REQUIRE_ATTRIBUTE);
        if(!found_trigger_name || !found_trigger_name_completed)
            return false;

        int count;
        bool reset_on_completed;
        FindAttribute(COUNT_ATTRIBUTE, properties, count, FallbackMode::SET_DEFAULT);
        FindAttribute(RESET_ON_COMPLETED_ATTRIBUTE, properties, reset_on_completed, FallbackMode::SET_DEFAULT);

        const uint32_t trigger_hash = hash::Hash(trigger_name.text.c_str());
        const uint32_t completed_trigger_hash = hash::Hash(trigger_name_completed.text.c_str());

        mono::TriggerSystem* trigger_system = context->GetSystem<mono::TriggerSystem>();
        trigger_system->AddCounterTrigger(entity->id, trigger_hash, completed_trigger_hash, count, reset_on_completed);

        hash::HashRegisterString(trigger_name.text.c_str());
        hash::HashRegisterString(trigger_name_completed.text.c_str());

        return true;
    }

    bool CreateRelayTrigger(mono::Entity* entity, mono::SystemContext* context)
    {
        mono::TriggerSystem* trigger_system = context->GetSystem<mono::TriggerSystem>();
        trigger_system->AllocateRelayTrigger(entity->id);
        return true;
    }

    bool ReleaseRelayTrigger(mono::Entity* entity, mono::SystemContext* context)
    {
        mono::TriggerSystem* trigger_system = context->GetSystem<mono::TriggerSystem>();
        trigger_system->ReleaseRelayTrigger(entity->id);
        return true;
    }

    bool UpdateRelayTrigger(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        mono::Event trigger_name;
        mono::Event trigger_name_completed;
        const bool found_trigger_name =
            FindAttribute(TRIGGER_NAME_ATTRIBUTE, properties, trigger_name, FallbackMode::REQUIRE_ATTRIBUTE);
        const bool found_trigger_name_completed =
            FindAttribute(TRIGGER_NAME_COMPLETED_ATTRIBUTE, properties, trigger_name_completed, FallbackMode::REQUIRE_ATTRIBUTE);
        if(!found_trigger_name || !found_trigger_name_completed)
            return false;

        int delay_ms;
        FindAttribute(TIME_STAMP_ATTRIBUTE, properties, delay_ms, FallbackMode::SET_DEFAULT);

        const uint32_t trigger_hash = hash::Hash(trigger_name.text.c_str());
        const uint32_t completed_trigger_hash = hash::Hash(trigger_name_completed.text.c_str());

        mono::TriggerSystem* trigger_system = context->GetSystem<mono::TriggerSystem>();
        trigger_system->AddRelayTrigger(entity->id, trigger_hash, completed_trigger_hash, delay_ms);

        hash::HashRegisterString(trigger_name.text.c_str());
        hash::HashRegisterString(trigger_name_completed.text.c_str());

        return true;
    }

    bool CreatePickup(mono::Entity* entity, mono::SystemContext* context)
    {
        game::PickupSystem* pickup_system = context->GetSystem<game::PickupSystem>();
        pickup_system->AllocatePickup(entity->id);
        return true;
    }

    bool ReleasePickup(mono::Entity* entity, mono::SystemContext* context)
    {
        game::PickupSystem* pickup_system = context->GetSystem<game::PickupSystem>();
        pickup_system->ReleasePickup(entity->id);
        return true;
    }

    bool UpdatePickup(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        game::Pickup pickup;
        FindAttribute(PICKUP_TYPE_ATTRIBUTE, properties, (int&)pickup.type, FallbackMode::SET_DEFAULT);
        FindAttribute(AMOUNT_ATTRIBUTE, properties, pickup.meta_data, FallbackMode::SET_DEFAULT);

        game::PickupSystem* pickup_system = context->GetSystem<game::PickupSystem>();
        pickup_system->SetPickupData(entity->id, pickup);

        return true;
    }

    bool UpdateWeaponPickup(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        game::Pickup pickup;
        pickup.type = game::PickupType::WEAPON_MODIFIER;
        FindAttribute(WEAPON_MODIFIER_TYPE_ATTRIBUTE, properties, pickup.meta_data, FallbackMode::SET_DEFAULT);

        game::PickupSystem* pickup_system = context->GetSystem<game::PickupSystem>();
        pickup_system->SetPickupData(entity->id, pickup);

        return true;
    }

    bool CreateLootBox(mono::Entity* entity, mono::SystemContext* context)
    {
        game::PickupSystem* pickup_system = context->GetSystem<game::PickupSystem>();
        pickup_system->AllocateLootBox(entity->id);
        return true;
    }
    
    bool ReleaseLootBox(mono::Entity* entity, mono::SystemContext* context)
    {
        game::PickupSystem* pickup_system = context->GetSystem<game::PickupSystem>();
        pickup_system->ReleaseLootBox(entity->id);
        return true;
    }
    
    bool UpdateLootBox(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        game::PickupSystem* pickup_system = context->GetSystem<game::PickupSystem>();
        pickup_system->SetLootBoxData(entity->id, 1.0f);
        return true;
    }

    bool CreateAnimation(mono::Entity* entity, mono::SystemContext* context)
    {
        game::AnimationSystem* animation_system = context->GetSystem<game::AnimationSystem>();
        animation_system->AllocateAnimationContainer(entity->id);
        return true;
    }

    bool ReleaseAnimation(mono::Entity* entity, mono::SystemContext* context)
    {
        game::AnimationSystem* animation_system = context->GetSystem<game::AnimationSystem>();
        animation_system->ReleaseAnimationContainer(entity->id);
        return true;
    }

    bool UpdateSpriteAnimation(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        mono::Event trigger_name;
        const bool found_trigger_name =
            FindAttribute(TRIGGER_NAME_ATTRIBUTE, properties, trigger_name, FallbackMode::REQUIRE_ATTRIBUTE);

        if(!found_trigger_name)
        {
            System::Log("GameComponentFunctions|Missing trigger name parameter, unable to update component");
            return false;
        }

        int animation_index;
        FindAttribute(ANIMATION_ATTRIBUTE, properties, animation_index, FallbackMode::SET_DEFAULT);

        game::AnimationSystem* animation_system = context->GetSystem<game::AnimationSystem>();
        animation_system->AddSpriteAnimation(entity->id, hash::Hash(trigger_name.text.c_str()), animation_index);

        return true;
    }

    bool UpdateTranslationAnimation(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        mono::Event trigger_name;
        const bool found_trigger_name =
            FindAttribute(TRIGGER_NAME_ATTRIBUTE, properties, trigger_name, FallbackMode::REQUIRE_ATTRIBUTE);

        if(!found_trigger_name)
        {
            System::Log("GameComponentFunctions|Missing trigger name parameter, unable to update component");
            return false;
        }

        math::Vector translation;
        float duration;
        int ease_func_index;
        int animation_mode;
        FindAttribute(POSITION_ATTRIBUTE, properties, translation, FallbackMode::SET_DEFAULT);
        FindAttribute(DURATION_ATTRIBUTE, properties, duration, FallbackMode::SET_DEFAULT);
        FindAttribute(EASING_FUNC_ATTRIBUTE, properties, ease_func_index, FallbackMode::SET_DEFAULT);
        FindAttribute(ANIMATION_MODE_ATTRIBUTE, properties, animation_mode, FallbackMode::SET_DEFAULT);

        game::AnimationSystem* animation_system = context->GetSystem<game::AnimationSystem>();
        animation_system->AddTranslationComponent(
            entity->id, hash::Hash(trigger_name.text.c_str()), duration, math::ease_functions[ease_func_index], math::ease_functions[ease_func_index], game::AnimationMode(animation_mode), translation);

        return true;
    }

    bool UpdateRotationAnimation(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        mono::Event trigger_name;
        const bool found_trigger_name =
            FindAttribute(TRIGGER_NAME_ATTRIBUTE, properties, trigger_name, FallbackMode::REQUIRE_ATTRIBUTE);

        if(!found_trigger_name)
        {
            System::Log("GameComponentFunctions|Missing trigger name parameter, unable to update component");
            return false;
        }

        float rotation;
        float duration;
        int ease_func_index;
        int animation_mode;
        FindAttribute(ROTATION_ATTRIBUTE, properties, rotation, FallbackMode::SET_DEFAULT);
        FindAttribute(DURATION_ATTRIBUTE, properties, duration, FallbackMode::SET_DEFAULT);
        FindAttribute(EASING_FUNC_ATTRIBUTE, properties, ease_func_index, FallbackMode::SET_DEFAULT);
        FindAttribute(ANIMATION_MODE_ATTRIBUTE, properties, animation_mode, FallbackMode::SET_DEFAULT);

        game::AnimationSystem* animation_system = context->GetSystem<game::AnimationSystem>();
        animation_system->AddRotationComponent(
            entity->id, hash::Hash(trigger_name.text.c_str()), duration, math::ease_functions[ease_func_index], game::AnimationMode(animation_mode), rotation);

        return true;
    }

    bool UpdateScaleAnimation(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        mono::Event trigger_name;
        const bool found_trigger_name =
            FindAttribute(TRIGGER_NAME_ATTRIBUTE, properties, trigger_name, FallbackMode::REQUIRE_ATTRIBUTE);

        if(!found_trigger_name)
        {
            System::Log("GameComponentFunctions|Missing trigger name parameter, unable to update component");
            return false;
        }

        float scale;
        float duration;
        int ease_func_index;
        int animation_mode;
        FindAttribute(SCALE_ATTRIBUTE, properties, scale, FallbackMode::SET_DEFAULT);
        FindAttribute(DURATION_ATTRIBUTE, properties, duration, FallbackMode::SET_DEFAULT);
        FindAttribute(EASING_FUNC_ATTRIBUTE, properties, ease_func_index, FallbackMode::SET_DEFAULT);
        FindAttribute(ANIMATION_MODE_ATTRIBUTE, properties, animation_mode, FallbackMode::SET_DEFAULT);

        game::AnimationSystem* animation_system = context->GetSystem<game::AnimationSystem>();
        animation_system->AddScaleComponent(
            entity->id, hash::Hash(trigger_name.text.c_str()), duration, math::ease_functions[ease_func_index], game::AnimationMode(animation_mode), scale);

        return true;
    }

    bool CreateCameraAnimation(mono::Entity* entity, mono::SystemContext* context)
    {
        game::CameraSystem* camera_system = context->GetSystem<game::CameraSystem>();
        camera_system->AllocateCameraAnimation(entity->id);
        return true;
    }

    bool ReleaseCameraAnimation(mono::Entity* entity, mono::SystemContext* context)
    {
        game::CameraSystem* camera_system = context->GetSystem<game::CameraSystem>();
        camera_system->ReleaseCameraAnimation(entity->id);
        return true;
    }

    bool UpdateCameraZoom(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        mono::Event trigger_name;
        const bool found_trigger_name =
            FindAttribute(TRIGGER_NAME_ATTRIBUTE, properties, trigger_name, FallbackMode::REQUIRE_ATTRIBUTE);
        if(!found_trigger_name)
        {
            System::Log("GameComponentFunctions|Missing trigger name parameter, unable to update component");
            return false;
        }

        float zoom_value;
        FindAttribute(ZOOM_LEVEL_ATTRIBUTE, properties, zoom_value, FallbackMode::SET_DEFAULT);

        game::CameraSystem* camera_system = context->GetSystem<game::CameraSystem>();
        camera_system->AddCameraAnimationComponent(entity->id, hash::Hash(trigger_name.text.c_str()), zoom_value);
        return true;
    }

    bool UpdateCameraPoint(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        mono::Event trigger_name;
        const bool found_trigger_name =
            FindAttribute(TRIGGER_NAME_ATTRIBUTE, properties, trigger_name, FallbackMode::REQUIRE_ATTRIBUTE);
        if(!found_trigger_name)
        {
            System::Log("GameComponentFunctions|Missing trigger name parameter, unable to update component");
            return false;
        }

        game::CameraSystem* camera_system = context->GetSystem<game::CameraSystem>();
        camera_system->AddCameraAnimationComponent(entity->id, hash::Hash(trigger_name.text.c_str()));
        return true;
    }

    bool UpdateCameraTrackEntity(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        mono::Event trigger_name;
        const bool found_trigger_name =
            FindAttribute(TRIGGER_NAME_ATTRIBUTE, properties, trigger_name, FallbackMode::REQUIRE_ATTRIBUTE);
        if(!found_trigger_name)
        {
            System::Log("GameComponentFunctions|Missing trigger name parameter, unable to update component");
            return false;
        }

        uint32_t entity_reference = 0;
        const bool found_path_property = FindAttribute(ENTITY_REFERENCE_ATTRIBUTE, properties, entity_reference, FallbackMode::REQUIRE_ATTRIBUTE);
        if(!found_path_property)
            return false;

        mono::IEntityManager* entity_manager = context->GetSystem<mono::IEntityManager>();
        const uint32_t referenced_entity_id = entity_manager->GetEntityIdFromUuid(entity_reference);

        game::CameraSystem* camera_system = context->GetSystem<game::CameraSystem>();
        camera_system->AddCameraAnimationComponent(entity->id, hash::Hash(trigger_name.text.c_str()), referenced_entity_id);
        return true;
    }

    bool CreateCameraRestore(mono::Entity* entity, mono::SystemContext* context)
    {
        game::CameraSystem* camera_system = context->GetSystem<game::CameraSystem>();
        camera_system->AllocateRestoreComponent(entity->id);
        return true;
    }

    bool ReleaseCameraRestore(mono::Entity* entity, mono::SystemContext* context)
    {
        game::CameraSystem* camera_system = context->GetSystem<game::CameraSystem>();
        camera_system->ReleaseRestoreComponent(entity->id);
        return true;
    }

    bool UpdateCameraRestore(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        mono::Event trigger_name;
        const bool found_trigger_name =
            FindAttribute(TRIGGER_NAME_ATTRIBUTE, properties, trigger_name, FallbackMode::REQUIRE_ATTRIBUTE);
        if(!found_trigger_name)
        {
            System::Log("GameComponentFunctions|Missing trigger name parameter, unable to update component");
            return false;
        }

        game::CameraSystem* camera_system = context->GetSystem<game::CameraSystem>();
        camera_system->AddRestoreComponent(entity->id, hash::Hash(trigger_name.text.c_str()));
        return true;
    }

    bool CreateInteraction(mono::Entity* entity, mono::SystemContext* context)
    {
        game::InteractionSystem* interaction_system = context->GetSystem<game::InteractionSystem>();
        interaction_system->AllocateComponent(entity->id);
        return true;
    }
    
    bool ReleaseInteraction(mono::Entity* entity, mono::SystemContext* context)
    {
        game::InteractionSystem* interaction_system = context->GetSystem<game::InteractionSystem>();
        interaction_system->ReleaseComponent(entity->id);
        return true;
    }
    
    bool UpdateInteraction(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        mono::Event trigger_name;
        const bool found_trigger_name =
            FindAttribute(TRIGGER_NAME_ATTRIBUTE, properties, trigger_name, FallbackMode::REQUIRE_ATTRIBUTE);
        if(!found_trigger_name)
        {
            System::Log("GameComponentFunctions|Missing trigger name parameter, unable to update component");
            return false;
        }

        game::InteractionType interaction_type;
        bool draw_name = false;
        std::string interaction_sound;

        FindAttribute(INTERACTION_TYPE_ATTRIBUTE, properties, (int&)interaction_type, FallbackMode::SET_DEFAULT);
        FindAttribute(DRAW_NAME_ATTRIBUTE, properties, draw_name, FallbackMode::SET_DEFAULT);
        FindAttribute(SOUND_ATTRIBUTE, properties, interaction_sound, FallbackMode::SET_DEFAULT);

        game::InteractionSystem* interaction_system = context->GetSystem<game::InteractionSystem>();
        interaction_system->AddComponent(
            entity->id,
            hash::Hash(trigger_name.text.c_str()),
            interaction_type,
            draw_name,
            interaction_sound);
        hash::HashRegisterString(trigger_name.text.c_str());

        return true;
    }

    bool UpdateInteractionSwitch(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        mono::Event on_trigger_name;
        mono::Event off_trigger_name;
        const bool found_trigger_name =
            FindAttribute(TRIGGER_NAME_ATTRIBUTE, properties, on_trigger_name, FallbackMode::REQUIRE_ATTRIBUTE) &&
            FindAttribute(TRIGGER_NAME_EXIT_ATTRIBUTE, properties, off_trigger_name, FallbackMode::REQUIRE_ATTRIBUTE);
        if(!found_trigger_name)
        {
            System::Log("GameComponentFunctions|Missing trigger name parameter, unable to update component");
            return false;
        }

        game::InteractionType interaction_type;
        bool draw_name = false;
        std::string interaction_sound;

        FindAttribute(INTERACTION_TYPE_ATTRIBUTE, properties, (int&)interaction_type, FallbackMode::SET_DEFAULT);
        FindAttribute(DRAW_NAME_ATTRIBUTE, properties, draw_name, FallbackMode::SET_DEFAULT);
        FindAttribute(SOUND_ATTRIBUTE, properties, interaction_sound, FallbackMode::SET_DEFAULT);

        game::InteractionSystem* interaction_system = context->GetSystem<game::InteractionSystem>();
        interaction_system->AddComponent(
            entity->id,
            hash::Hash(on_trigger_name.text.c_str()),
            hash::Hash(off_trigger_name.text.c_str()),
            interaction_type,
            draw_name,
            interaction_sound);

        hash::HashRegisterString(on_trigger_name.text.c_str());
        hash::HashRegisterString(off_trigger_name.text.c_str());

        return true;
    }

    bool CreateDialog(mono::Entity* entity, mono::SystemContext* context)
    {
        game::DialogSystem* dialog_system = context->GetSystem<game::DialogSystem>();
        dialog_system->AllocateComponent(entity->id);
        return true;
    }
    
    bool ReleaseDialog(mono::Entity* entity, mono::SystemContext* context)
    {
        game::DialogSystem* dialog_system = context->GetSystem<game::DialogSystem>();
        dialog_system->ReleaseComponent(entity->id);
        return true;
    }
    
    bool UpdateDialog(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        std::string message;
        const bool found_message = FindAttribute(TEXT_ATTRIBUTE, properties, message, FallbackMode::REQUIRE_ATTRIBUTE);
        if(!found_message)
            return false;

        float duration;
        FindAttribute(DURATION_ATTRIBUTE, properties, duration, FallbackMode::SET_DEFAULT);

        game::DialogSystem* dialog_system = context->GetSystem<game::DialogSystem>();
        dialog_system->AddComponent(entity->id, message, duration);

        return true;
    }

    bool CreateNothing(mono::Entity* entity, mono::SystemContext* context)
    {
        return true;
    }
    bool DestroyNothing(mono::Entity* entity, mono::SystemContext* context)
    {
        return true;
    }

    bool UpdateWeaponLoadout(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        std::string primary;
        std::string secondary;
        std::string tertiary;
        FindAttribute(WEAPON_PRIMARY_ATTRIBUTE, properties, primary, FallbackMode::SET_DEFAULT);
        FindAttribute(WEAPON_SECONDARY_ATTRIBUTE, properties, secondary, FallbackMode::SET_DEFAULT);
        FindAttribute(WEAPON_TERTIARY_ATTRIBUTE, properties, tertiary, FallbackMode::SET_DEFAULT);

        game::WeaponSystem* weapon_system = context->GetSystem<game::WeaponSystem>();
        weapon_system->SetWeaponLoadout(entity->id, primary, secondary, tertiary);

        return true;
    }

    bool CreateSound(mono::Entity* entity, mono::SystemContext* context)
    {
        game::SoundSystem* sound_system = context->GetSystem<game::SoundSystem>();
        sound_system->AllocateSoundComponent(entity->id);
        return true;
    }

    bool ReleaseSound(mono::Entity* entity, mono::SystemContext* context)
    {
        game::SoundSystem* sound_system = context->GetSystem<game::SoundSystem>();
        sound_system->ReleaseSoundComponent(entity->id);
        return true;
    }

    bool UpdateSound(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        std::string sound_file;
        mono::Event play_trigger;
        mono::Event stop_trigger;
        uint32_t parameters = 0;

        FindAttribute(SOUND_ATTRIBUTE, properties, sound_file, FallbackMode::SET_DEFAULT);
        FindAttribute(SOUND_PLAY_PARAMETERS, properties, parameters, FallbackMode::SET_DEFAULT);
        FindAttribute(ENABLE_TRIGGER_ATTRIBUTE, properties, play_trigger, FallbackMode::SET_DEFAULT);
        FindAttribute(DISABLE_TRIGGER_ATTRIBUTE, properties, stop_trigger, FallbackMode::SET_DEFAULT);

        game::SoundSystem* sound_system = context->GetSystem<game::SoundSystem>();
        sound_system->SetSoundComponentData(
            entity->id,
            sound_file,
            game::SoundInstancePlayParameter(parameters),
            hash::Hash(play_trigger.text.c_str()),
            hash::Hash(stop_trigger.text.c_str()));
        return true;
    }


    bool CreateTeleportPlayer(mono::Entity* entity, mono::SystemContext* context)
    {
        game::TeleportSystem* teleport_system = context->GetSystem<game::TeleportSystem>();
        teleport_system->AllocateTeleportPlayer(entity->id);
        return true;
    }

    bool ReleaseTeleportPlayer(mono::Entity* entity, mono::SystemContext* context)
    {
        game::TeleportSystem* teleport_system = context->GetSystem<game::TeleportSystem>();
        teleport_system->ReleaseTeleportPlayer(entity->id);
        return true;
    }

    bool UpdateTeleportPlayer(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        mono::Event trigger_name;
        FindAttribute(TRIGGER_NAME_ATTRIBUTE, properties, trigger_name, FallbackMode::SET_DEFAULT);

        game::TeleportSystem* teleport_system = context->GetSystem<game::TeleportSystem>();
        teleport_system->UpdateTeleportPlayer(entity->id, hash::Hash(trigger_name.text.c_str()));

        return true;
    }

    bool CreateEntityTracker(mono::Entity* entity, mono::SystemContext* context)
    {
        game::WorldEntityTrackingSystem* tracking_system = context->GetSystem<game::WorldEntityTrackingSystem>();
        tracking_system->AllocateEntityTracker(entity->id);
        return true;
    }

    bool ReleaseEntityTracker(mono::Entity* entity, mono::SystemContext* context)
    {
        game::WorldEntityTrackingSystem* tracking_system = context->GetSystem<game::WorldEntityTrackingSystem>();
        tracking_system->ReleaseEntityTracker(entity->id);
        return true;
    }

    bool UpdateEntityTracker(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        int entity_type;
        FindAttribute(ENTITY_TYPE_ATTRIBUTE, properties, entity_type, FallbackMode::SET_DEFAULT);

        game::WorldEntityTrackingSystem* tracking_system = context->GetSystem<game::WorldEntityTrackingSystem>();
        tracking_system->UpdateEntityTracker(entity->id, game::EntityType(entity_type));

        return true;
    }

    bool CreateEntityTarget(mono::Entity* entity, mono::SystemContext* context)
    {
        game::TargetSystem* target_system = context->GetSystem<game::TargetSystem>();
        target_system->AllocateTarget(entity->id);
        return true;
    }

    bool ReleaseEntityTarget(mono::Entity* entity, mono::SystemContext* context)
    {
        game::TargetSystem* target_system = context->GetSystem<game::TargetSystem>();
        target_system->ReleaseTarget(entity->id);
        return true;
    }

    bool UpdateEntityTarget(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        int faction;
        int priority;
        FindAttribute(TARGET_FACTION_ATTRIBUTE, properties, faction, FallbackMode::SET_DEFAULT);
        FindAttribute(PRIORITY_ATTRIBUTE, properties, priority, FallbackMode::SET_DEFAULT);

        game::TargetSystem* target_system = context->GetSystem<game::TargetSystem>();
        target_system->SetTargetData(entity->id, game::TargetFaction(faction), priority);
        return true;
    }

    bool CreateMissionTracker(mono::Entity* entity, mono::SystemContext* context)
    {
        game::MissionSystem* mission_system = context->GetSystem<game::MissionSystem>();
        mission_system->AllocateMission(entity->id);
        return true;
    }
    bool ReleaseMissionTracker(mono::Entity* entity, mono::SystemContext* context)
    {
        game::MissionSystem* mission_system = context->GetSystem<game::MissionSystem>();
        mission_system->ReleaseMission(entity->id);
        return true;
    }
    bool UpdateMissionTracker(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        std::string mission_name;
        std::string mission_description;
        bool time_based;
        float time_s;
        bool fail_on_timeout;
        FindAttribute(NAME_ATTRIBUTE, properties, mission_name, FallbackMode::SET_DEFAULT);
        FindAttribute(SUB_TEXT_ATTRIBUTE, properties, mission_description, FallbackMode::SET_DEFAULT);
        FindAttribute(TIME_BASED_ATTRIBUTE, properties, time_based, FallbackMode::SET_DEFAULT);
        FindAttribute(TIME_ATTRIBUTE, properties, time_s, FallbackMode::SET_DEFAULT);
        FindAttribute(FAIL_ON_TIMEOUT_ATTRIBUTE, properties, fail_on_timeout, FallbackMode::SET_DEFAULT);

        mono::Event trigger_name;
        mono::Event completed_trigger_name;
        mono::Event failed_trigger_name;
        FindAttribute(TRIGGER_NAME_ATTRIBUTE, properties, trigger_name, FallbackMode::SET_DEFAULT);
        FindAttribute(COMPLETED_TRIGGER_ATTRIBUTE, properties, completed_trigger_name, FallbackMode::SET_DEFAULT);
        FindAttribute(FAILED_TRIGGER_ATTRIBUTE, properties, failed_trigger_name, FallbackMode::SET_DEFAULT);

        game::MissionSystem* mission_system = context->GetSystem<game::MissionSystem>();
        mission_system->SetMissionData(
            entity->id,
            mission_name,
            mission_description,
            time_based,
            time_s,
            fail_on_timeout,
            hash::Hash(trigger_name.text.c_str()),
            hash::Hash(completed_trigger_name.text.c_str()),
            hash::Hash(failed_trigger_name.text.c_str()));

        return true;
    }

    bool CreateMissionActivator(mono::Entity* entity, mono::SystemContext* context)
    {
        game::MissionSystem* mission_system = context->GetSystem<game::MissionSystem>();
        mission_system->AllocateMissionActivator(entity->id);
        return true;
    }
    bool ReleaseMissionActivator(mono::Entity* entity, mono::SystemContext* context)
    {
        game::MissionSystem* mission_system = context->GetSystem<game::MissionSystem>();
        mission_system->ReleaseMissionActivator(entity->id);
        return true;
    }
    bool UpdateMissionActivator(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        mono::Event trigger_name;
        bool emit_once;
        FindAttribute(TRIGGER_NAME_ATTRIBUTE, properties, trigger_name, FallbackMode::SET_DEFAULT);
        FindAttribute(EMIT_ONCE_ATTRIBUTE, properties, emit_once, FallbackMode::SET_DEFAULT);

        game::MissionSystem* mission_system = context->GetSystem<game::MissionSystem>();
        mission_system->SetMissionActivatorData(
            entity->id, hash::Hash(trigger_name.text.c_str()), emit_once);

        return true;
    }

    bool CreateMissionLocation(mono::Entity* entity, mono::SystemContext* context)
    {
        game::MissionSystem* mission_system = context->GetSystem<game::MissionSystem>();
        mission_system->AllocateMissionLocation(entity->id);
        return true;
    }
    bool ReleaseMissionLocation(mono::Entity* entity, mono::SystemContext* context)
    {
        game::MissionSystem* mission_system = context->GetSystem<game::MissionSystem>();
        mission_system->ReleaseMissionLocation(entity->id);
        return true;
    }

    bool CreatePhysicsImpulse(mono::Entity* entity, mono::SystemContext* context)
    {
        game::GamePhysicsSystem* game_physics = context->GetSystem<game::GamePhysicsSystem>();
        game_physics->AllocateImpulse(entity->id);
        return true;
    }

    bool ReleasePhysicsImpulse(mono::Entity* entity, mono::SystemContext* context)
    {
        game::GamePhysicsSystem* game_physics = context->GetSystem<game::GamePhysicsSystem>();
        game_physics->ReleaseImpulse(entity->id);
        return true;
    }

    bool UpdatePhysicsImpulse(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        float impulse_strength = 1.0f;
        FindAttribute(STRENGTH_ATTRIBUTE, properties, impulse_strength, FallbackMode::SET_DEFAULT);

        game::GamePhysicsSystem* game_physics = context->GetSystem<game::GamePhysicsSystem>();
        game_physics->UpdateImpulse(entity->id, impulse_strength);

        return true;
    }
}

void game::RegisterGameComponents(mono::IEntityManager* entity_manager)
{
    entity_manager->RegisterComponent(PHYSICS_COMPONENT, CreatePhysics, ReleasePhysics, UpdatePhysics);
    entity_manager->RegisterComponent(CIRCLE_SHAPE_COMPONENT, CreateShape, ReleaseShape, UpdateCircleShape);
    entity_manager->RegisterComponent(BOX_SHAPE_COMPONENT, CreateShape, ReleaseShape, UpdateBoxShape);
    entity_manager->RegisterComponent(SEGMENT_SHAPE_COMPONENT, CreateShape, ReleaseShape, UpdateSegmentShape);
    entity_manager->RegisterComponent(POLYGON_SHAPE_COMPONENT, CreateShape, ReleaseShape, UpdatePolygonShape);
    entity_manager->RegisterComponent(HEALTH_COMPONENT, CreateHealth, ReleaseHealth, UpdateHealth);
    entity_manager->RegisterComponent(SHOCKWAVE_COMPONENT, CreateShockwave, ReleaseShockwave, UpdateShockwave);
    entity_manager->RegisterComponent(BEHAVIOUR_COMPONENT, CreateEntityLogic, ReleaseEntityLogic, UpdateEntityLogic);
    entity_manager->RegisterComponent(SPAWN_POINT_COMPONENT, CreateSpawnPoint, ReleaseSpawnPoint, UpdateSpawnPoint);
    entity_manager->RegisterComponent(ENTITY_SPAWN_POINT_COMPONENT, CreateEntitySpawnPoint, ReleaseEntitySpawnPoint, UpdateEntitySpawnPoint);
    entity_manager->RegisterComponent(DESPAWN_ENTITY_COMPONENT, CreateDespawnEntity, ReleaseDespawnEntity, UpdateDespawnEntity);
    entity_manager->RegisterComponent(SHAPE_TRIGGER_COMPONENT, CreateShapeTrigger, ReleaseShapeTrigger, UpdateShapeTrigger);
    entity_manager->RegisterComponent(DESTROYED_TRIGGER_COMPONENT, CreateDestroyedTrigger, ReleaseDestroyedTrigger, UpdateDestroyedTrigger);
    entity_manager->RegisterComponent(AREA_TRIGGER_COMPONENT, CreateAreaTrigger, ReleaseAreaTrigger, UpdateAreaTrigger);
    entity_manager->RegisterComponent(TIME_TRIGGER_COMPONENT, CreateTimeTrigger, ReleaseTimeTrigger, UpdateTimeTrigger);
    entity_manager->RegisterComponent(COUNTER_TRIGGER_COMPONENT, CreateCounterTrigger, ReleaseCounterTrigger, UpdateCounterTrigger);
    entity_manager->RegisterComponent(RELAY_TRIGGER_COMPONENT, CreateRelayTrigger, ReleaseRelayTrigger, UpdateRelayTrigger);
    entity_manager->RegisterComponent(PICKUP_COMPONENT, CreatePickup, ReleasePickup, UpdatePickup);
    entity_manager->RegisterComponent(WEAPON_PICKUP_COMPONENT, CreatePickup, ReleasePickup, UpdateWeaponPickup);
    entity_manager->RegisterComponent(LOOTBOX_COMPONENT, CreateLootBox, ReleaseLootBox, UpdateLootBox);
    entity_manager->RegisterComponent(ANIMATION_COMPONENT, CreateAnimation, ReleaseAnimation, UpdateSpriteAnimation);
    entity_manager->RegisterComponent(TRANSLATION_COMPONENT, CreateAnimation, ReleaseAnimation, UpdateTranslationAnimation);
    entity_manager->RegisterComponent(ROTATION_COMPONENT, CreateAnimation, ReleaseAnimation, UpdateRotationAnimation);
    entity_manager->RegisterComponent(SCALE_COMPONENT, CreateAnimation, ReleaseAnimation, UpdateScaleAnimation);
    entity_manager->RegisterComponent(CAMERA_ZOOM_COMPONENT, CreateCameraAnimation, ReleaseCameraAnimation, UpdateCameraZoom);
    entity_manager->RegisterComponent(CAMERA_POINT_COMPONENT, CreateCameraAnimation, ReleaseCameraAnimation, UpdateCameraPoint);
    entity_manager->RegisterComponent(CAMERA_TRACK_ENTITY_COMPONENT, CreateCameraAnimation, ReleaseCameraAnimation, UpdateCameraTrackEntity);
    entity_manager->RegisterComponent(CAMERA_RESTORE_COMPONENT, CreateCameraRestore, ReleaseCameraRestore, UpdateCameraRestore);
    entity_manager->RegisterComponent(INTERACTION_COMPONENT, CreateInteraction, ReleaseInteraction, UpdateInteraction);
    entity_manager->RegisterComponent(INTERACTION_SWITCH_COMPONENT, CreateInteraction, ReleaseInteraction, UpdateInteractionSwitch);
    entity_manager->RegisterComponent(DIALOG_COMPONENT, CreateDialog, ReleaseDialog, UpdateDialog);
    entity_manager->RegisterComponent(WEAPON_LOADOUT_COMPONENT, CreateNothing, DestroyNothing, UpdateWeaponLoadout);
    entity_manager->RegisterComponent(SOUND_COMPONENT, CreateSound, ReleaseSound, UpdateSound);
    entity_manager->RegisterComponent(TELEPORT_PLAYER_COMPONENT, CreateTeleportPlayer, ReleaseTeleportPlayer, UpdateTeleportPlayer);
    entity_manager->RegisterComponent(ENTITY_TRACKING_COMPONENT, CreateEntityTracker, ReleaseEntityTracker, UpdateEntityTracker);
    entity_manager->RegisterComponent(TARGET_COMPONENT, CreateEntityTarget, ReleaseEntityTarget, UpdateEntityTarget);
    entity_manager->RegisterComponent(MISSION_TRACKER_COMPONENT, CreateMissionTracker, ReleaseMissionTracker, UpdateMissionTracker);
    entity_manager->RegisterComponent(MISSION_ACTIVATION_COMPONENT, CreateMissionActivator, ReleaseMissionActivator, UpdateMissionActivator);
    entity_manager->RegisterComponent(MISSION_LOCATION_COMPONENT, CreateMissionLocation, ReleaseMissionLocation);
    entity_manager->RegisterComponent(PHYSICS_IMPULSE_COMPONENT, CreatePhysicsImpulse, ReleasePhysicsImpulse, UpdatePhysicsImpulse);
}
