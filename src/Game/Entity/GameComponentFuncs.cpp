
#include "GameComponentFuncs.h"

#include "SystemContext.h"
#include "TransformSystem/TransformSystem.h"
#include "Factories.h"

#include "DamageSystem.h"
#include "TriggerSystem/TriggerSystem.h"
#include "Pickups/PickupSystem.h"
#include "SpawnSystem/SpawnSystem.h"
#include "EntitySystem/IEntityManager.h"
#include "Entity/EntityLogicSystem.h"
#include "Entity/EntityLogicFactory.h"
#include "Entity/AnimationSystem.h"
#include "GameCamera/CameraSystem.h"
#include "InteractionSystem/InteractionSystem.h"
#include "DialogSystem/DialogSystem.h"

#include "Physics/PhysicsSystem.h"
#include "Pickups/PickupSystem.h"

#include "Component.h"
#include "CollisionConfiguration.h"

#include "System/Hash.h"
#include "System/System.h"
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

        mono::PhysicsSystem* physics_system = context->GetSystem<mono::PhysicsSystem>();
        mono::IBody* body = physics_system->GetBody(entity->id);
        body->SetMass(body_args.mass);
        body->SetPreventRotation(prevent_rotation);
        body->SetMoment(body_args.inertia);
        body->SetType(body_args.type);

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

        const shared::FactionPair& faction_pair = shared::faction_lookup_table[faction];
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

        const shared::FactionPair& faction_pair = shared::faction_lookup_table[faction];
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

        const shared::FactionPair& faction_pair = shared::faction_lookup_table[faction];
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

        const shared::FactionPair& faction_pair = shared::faction_lookup_table[faction];
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
        FindAttribute(HEALTH_ATTRIBUTE, properties, health, FallbackMode::SET_DEFAULT);

        bool is_boss_health;
        FindAttribute(BOSS_HEALTH_ATTRIBUTE, properties, is_boss_health, FallbackMode::SET_DEFAULT);

        game::DamageSystem* damage_system = context->GetSystem<game::DamageSystem>();
        game::DamageRecord* damage_record = damage_system->GetDamageRecord(entity->id);
        damage_record->health = health;
        damage_record->full_health = health;
        damage_record->is_boss = is_boss_health;

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

        game::IEntityLogic* entity_logic = game::g_logic_factory->CreateLogic(shared::EntityLogicType(logic_type_value), properties, entity->id);
        game::EntityLogicSystem* logic_system = context->GetSystem<game::EntityLogicSystem>();
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
        FindAttribute(RADIUS_ATTRIBUTE, properties, spawn_point.radius, FallbackMode::SET_DEFAULT);
        FindAttribute(TIME_STAMP_ATTRIBUTE, properties, spawn_point.interval, FallbackMode::SET_DEFAULT);

        std::string enable_trigger;
        const bool found_enable = FindAttribute(ENABLE_TRIGGER_ATTRIBUTE, properties, enable_trigger, FallbackMode::SET_DEFAULT);
        if(found_enable)
            spawn_point.enable_trigger = hash::Hash(enable_trigger.c_str());

        std::string disable_trigger;
        const bool found_disable = FindAttribute(DISABLE_TRIGGER_ATTRIBUTE, properties, disable_trigger, FallbackMode::SET_DEFAULT);
        if(found_disable)
            spawn_point.disable_trigger = hash::Hash(disable_trigger.c_str());

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

        std::string spawn_trigger_name;
        const bool found_enable = FindAttribute(TRIGGER_NAME_ATTRIBUTE, properties, spawn_trigger_name, FallbackMode::SET_DEFAULT);
        if(found_enable)
            spawn_trigger = hash::Hash(spawn_trigger_name.c_str());

        game::SpawnSystem* spawn_system = context->GetSystem<game::SpawnSystem>();
        spawn_system->SetEntitySpawnPointData(entity->id, entity_file, spawn_radius, spawn_trigger);
        return true;
    }

    bool CreateShapeTrigger(mono::Entity* entity, mono::SystemContext* context)
    {
        game::TriggerSystem* trigger_system = context->GetSystem<game::TriggerSystem>();
        trigger_system->AllocateShapeTrigger(entity->id);
        return true;
    }
    
    bool ReleaseShapeTrigger(mono::Entity* entity, mono::SystemContext* context)
    {
        game::TriggerSystem* trigger_system = context->GetSystem<game::TriggerSystem>();
        trigger_system->ReleaseShapeTrigger(entity->id);
        return true;
    }
    
    bool UpdateShapeTrigger(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        std::string enter_trigger_name;
        const bool found_trigger_name =
            FindAttribute(TRIGGER_NAME_ATTRIBUTE, properties, enter_trigger_name, FallbackMode::REQUIRE_ATTRIBUTE);

        std::string exit_trigger_name;
        const bool found_exit_trigger_name =
            FindAttribute(TRIGGER_NAME_EXIT_ATTRIBUTE, properties, exit_trigger_name, FallbackMode::REQUIRE_ATTRIBUTE);

        if(!found_trigger_name && !found_exit_trigger_name)
        {
            System::Log("GameComponentFunctions|Missing trigger name parameter, unable to update component");
            return false;
        }

        uint32_t faction;
        FindAttribute(FACTION_PICKER_ATTRIBUTE, properties, faction, FallbackMode::SET_DEFAULT);

        const uint32_t enter_trigger_hash = hash::Hash(enter_trigger_name.c_str());
        const uint32_t exit_trigger_hash = hash::Hash(exit_trigger_name.c_str());

        game::TriggerSystem* trigger_system = context->GetSystem<game::TriggerSystem>();
        trigger_system->AddShapeTrigger(entity->id, enter_trigger_hash, exit_trigger_hash, faction);

        hash::HashRegisterString(enter_trigger_name.c_str());
        hash::HashRegisterString(exit_trigger_name.c_str());

        return true;
    }

    bool CreateDestroyedTrigger(mono::Entity* entity, mono::SystemContext* context)
    {
        game::TriggerSystem* trigger_system = context->GetSystem<game::TriggerSystem>();
        trigger_system->AllocateDestroyedTrigger(entity->id);
        return true;
    }

    bool ReleaseDestroyedTrigger(mono::Entity* entity, mono::SystemContext* context)
    {
        game::TriggerSystem* trigger_system = context->GetSystem<game::TriggerSystem>();
        trigger_system->ReleaseDestroyedTrigger(entity->id);
        return true;
    }

    bool UpdateDestroyedTrigger(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        std::string trigger_name;
        const bool found_trigger_name =
            FindAttribute(TRIGGER_NAME_ATTRIBUTE, properties, trigger_name, FallbackMode::REQUIRE_ATTRIBUTE);

        if(!found_trigger_name)
        {
            System::Log("GameComponentFunctions|Missing trigger name parameter, unable to update component");
            return false;
        }

        int trigger_type;
        FindAttribute(DESTROYED_TRIGGER_TYPE_ATTRIBUTE, properties, trigger_type, FallbackMode::SET_DEFAULT);

        const uint32_t trigger_hash = hash::Hash(trigger_name.c_str());
        game::TriggerSystem* trigger_system = context->GetSystem<game::TriggerSystem>();
        trigger_system->AddDestroyedTrigger(entity->id, trigger_hash, shared::DestroyedTriggerType(trigger_type));
        hash::HashRegisterString(trigger_name.c_str());

        return true;
    }

    bool CreateAreaTrigger(mono::Entity* entity, mono::SystemContext* context)
    {
        game::TriggerSystem* trigger_system = context->GetSystem<game::TriggerSystem>();
        trigger_system->AllocateAreaTrigger(entity->id);
        return true;
    }

    bool ReleaseAreaTrigger(mono::Entity* entity, mono::SystemContext* context)
    {
        game::TriggerSystem* trigger_system = context->GetSystem<game::TriggerSystem>();
        trigger_system->ReleaseAreaTrigger(entity->id);
        return true;
    }

    bool UpdateAreaTrigger(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        std::string trigger_name;
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
        const math::Quad world_bb = math::Transform(world_transform, math::Quad(-half_width_height, half_width_height));

        const uint32_t trigger_hash = hash::Hash(trigger_name.c_str());

        game::TriggerSystem* trigger_system = context->GetSystem<game::TriggerSystem>();
        trigger_system->AddAreaEntityTrigger(
            entity->id, trigger_hash, world_bb, faction, shared::AreaTriggerOperation(operation), n_entities);
        hash::HashRegisterString(trigger_name.c_str());

        return true;
    }

    bool CreateTimeTrigger(mono::Entity* entity, mono::SystemContext* context)
    {
        game::TriggerSystem* trigger_system = context->GetSystem<game::TriggerSystem>();
        trigger_system->AllocateTimeTrigger(entity->id);
        return true;
    }

    bool ReleaseTimeTrigger(mono::Entity* entity, mono::SystemContext* context)
    {
        game::TriggerSystem* trigger_system = context->GetSystem<game::TriggerSystem>();
        trigger_system->ReleaseTimeTrigger(entity->id);
        return true;
    }

    bool UpdateTimeTrigger(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        std::string trigger_name;
        const bool found_trigger_name =
            FindAttribute(TRIGGER_NAME_ATTRIBUTE, properties, trigger_name, FallbackMode::REQUIRE_ATTRIBUTE);
        if(!found_trigger_name)
            return false;

        int timeout_ms;
        bool repeating;
        FindAttribute(TIME_STAMP_ATTRIBUTE, properties, timeout_ms, FallbackMode::SET_DEFAULT);
        FindAttribute(REPEATING_ATTRIBUTE, properties, repeating, FallbackMode::SET_DEFAULT);

        const uint32_t trigger_hash = hash::Hash(trigger_name.c_str());
        game::TriggerSystem* trigger_system = context->GetSystem<game::TriggerSystem>();
        trigger_system->AddTimeTrigger(entity->id, trigger_hash, timeout_ms, repeating);
        hash::HashRegisterString(trigger_name.c_str());

        return true;
    }

    bool CreateCounterTrigger(mono::Entity* entity, mono::SystemContext* context)
    {
        game::TriggerSystem* trigger_system = context->GetSystem<game::TriggerSystem>();
        trigger_system->AllocateCounterTrigger(entity->id);
        return true;
    }

    bool ReleaseCounterTrigger(mono::Entity* entity, mono::SystemContext* context)
    {
        game::TriggerSystem* trigger_system = context->GetSystem<game::TriggerSystem>();
        trigger_system->ReleaseCounterTrigger(entity->id);
        return true;
    }

    bool UpdateCounterTrigger(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        std::string trigger_name;
        std::string trigger_name_completed;
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

        const uint32_t trigger_hash = hash::Hash(trigger_name.c_str());
        const uint32_t completed_trigger_hash = hash::Hash(trigger_name_completed.c_str());

        game::TriggerSystem* trigger_system = context->GetSystem<game::TriggerSystem>();
        trigger_system->AddCounterTrigger(entity->id, trigger_hash, completed_trigger_hash, count, reset_on_completed);

        hash::HashRegisterString(trigger_name.c_str());
        hash::HashRegisterString(trigger_name_completed.c_str());

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
        FindAttribute(AMOUNT_ATTRIBUTE, properties, pickup.amount, FallbackMode::SET_DEFAULT);

        game::PickupSystem* pickup_system = context->GetSystem<game::PickupSystem>();
        pickup_system->SetPickupData(entity->id, pickup);

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

    bool UpdateAnimation(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        std::string trigger_name;
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
        animation_system->AddSpriteAnimation(entity->id, hash::Hash(trigger_name.c_str()), animation_index);

        return true;
    }

    bool CreateTranslation(mono::Entity* entity, mono::SystemContext* context)
    {
        game::AnimationSystem* animation_system = context->GetSystem<game::AnimationSystem>();
        animation_system->AllocateAnimationContainer(entity->id);
        return true;
    }

    bool ReleaseTranslation(mono::Entity* entity, mono::SystemContext* context)
    {
        game::AnimationSystem* animation_system = context->GetSystem<game::AnimationSystem>();
        animation_system->ReleaseAnimationContainer(entity->id);
        return true;
    }

    bool UpdateTranslation(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        std::string trigger_name;
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
            entity->id, hash::Hash(trigger_name.c_str()), duration, math::ease_functions[ease_func_index], shared::AnimationMode(animation_mode), translation);

        return true;
    }

    bool CreateRotation(mono::Entity* entity, mono::SystemContext* context)
    {
        game::AnimationSystem* animation_system = context->GetSystem<game::AnimationSystem>();
        animation_system->AllocateAnimationContainer(entity->id);
        return true;
    }

    bool ReleaseRotation(mono::Entity* entity, mono::SystemContext* context)
    {
        game::AnimationSystem* animation_system = context->GetSystem<game::AnimationSystem>();
        animation_system->ReleaseAnimationContainer(entity->id);
        return true;
    }

    bool UpdateRotation(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        std::string trigger_name;
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
            entity->id, hash::Hash(trigger_name.c_str()), duration, math::ease_functions[ease_func_index], shared::AnimationMode(animation_mode), rotation);

        return true;
    }

    bool CreateCameraZoom(mono::Entity* entity, mono::SystemContext* context)
    {
        game::CameraSystem* camera_system = context->GetSystem<game::CameraSystem>();
        camera_system->AllocateCameraAnimation(entity->id);
        return true;
    }

    bool ReleaseCameraZoom(mono::Entity* entity, mono::SystemContext* context)
    {
        game::CameraSystem* camera_system = context->GetSystem<game::CameraSystem>();
        camera_system->ReleaseCameraAnimation(entity->id);
        return true;
    }

    bool UpdateCameraZoom(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        std::string trigger_name;
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
        camera_system->AddCameraAnimationComponent(entity->id, hash::Hash(trigger_name.c_str()), zoom_value);
        return true;
    }

    bool CreateCameraPoint(mono::Entity* entity, mono::SystemContext* context)
    {
        game::CameraSystem* camera_system = context->GetSystem<game::CameraSystem>();
        camera_system->AllocateCameraAnimation(entity->id);
        return true;
    }

    bool ReleaseCameraPoint(mono::Entity* entity, mono::SystemContext* context)
    {
        game::CameraSystem* camera_system = context->GetSystem<game::CameraSystem>();
        camera_system->ReleaseCameraAnimation(entity->id);
        return true;
    }

    bool UpdateCameraPoint(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        std::string trigger_name;
        const bool found_trigger_name =
            FindAttribute(TRIGGER_NAME_ATTRIBUTE, properties, trigger_name, FallbackMode::REQUIRE_ATTRIBUTE);
        if(!found_trigger_name)
        {
            System::Log("GameComponentFunctions|Missing trigger name parameter, unable to update component");
            return false;
        }

        math::Vector point;
        FindAttribute(POSITION_ATTRIBUTE, properties, point, FallbackMode::SET_DEFAULT);

        game::CameraSystem* camera_system = context->GetSystem<game::CameraSystem>();
        camera_system->AddCameraAnimationComponent(entity->id, hash::Hash(trigger_name.c_str()), point);
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
        std::string trigger_name;
        const bool found_trigger_name =
            FindAttribute(TRIGGER_NAME_ATTRIBUTE, properties, trigger_name, FallbackMode::REQUIRE_ATTRIBUTE);
        if(!found_trigger_name)
        {
            System::Log("GameComponentFunctions|Missing trigger name parameter, unable to update component");
            return false;
        }

        game::CameraSystem* camera_system = context->GetSystem<game::CameraSystem>();
        camera_system->AddRestoreComponent(entity->id, hash::Hash(trigger_name.c_str()));
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
        std::string trigger_name;
        const bool found_trigger_name =
            FindAttribute(TRIGGER_NAME_ATTRIBUTE, properties, trigger_name, FallbackMode::REQUIRE_ATTRIBUTE);
        if(!found_trigger_name)
        {
            System::Log("GameComponentFunctions|Missing trigger name parameter, unable to update component");
            return false;
        }

        shared::InteractionType interaction_type;
        FindAttribute(INTERACTION_TYPE_ATTRIBUTE, properties, (int&)interaction_type, FallbackMode::SET_DEFAULT);

        bool draw_name = false;
        FindAttribute(DRAW_NAME_ATTRIBUTE, properties, draw_name, FallbackMode::SET_DEFAULT);

        game::InteractionSystem* interaction_system = context->GetSystem<game::InteractionSystem>();
        interaction_system->AddComponent(entity->id, hash::Hash(trigger_name.c_str()), interaction_type, draw_name);
        hash::HashRegisterString(trigger_name.c_str());

        return true;
    }

    bool UpdateInteractionSwitch(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        std::string on_trigger_name;
        std::string off_trigger_name;
        const bool found_trigger_name =
            FindAttribute(TRIGGER_NAME_ATTRIBUTE, properties, on_trigger_name, FallbackMode::REQUIRE_ATTRIBUTE) &&
            FindAttribute(TRIGGER_NAME_EXIT_ATTRIBUTE, properties, off_trigger_name, FallbackMode::REQUIRE_ATTRIBUTE);
        if(!found_trigger_name)
        {
            System::Log("GameComponentFunctions|Missing trigger name parameter, unable to update component");
            return false;
        }

        shared::InteractionType interaction_type;
        FindAttribute(INTERACTION_TYPE_ATTRIBUTE, properties, (int&)interaction_type, FallbackMode::SET_DEFAULT);

        bool draw_name = false;
        FindAttribute(DRAW_NAME_ATTRIBUTE, properties, draw_name, FallbackMode::SET_DEFAULT);

        game::InteractionSystem* interaction_system = context->GetSystem<game::InteractionSystem>();
        interaction_system->AddComponent(
            entity->id, hash::Hash(on_trigger_name.c_str()), hash::Hash(off_trigger_name.c_str()), interaction_type, draw_name);

        hash::HashRegisterString(on_trigger_name.c_str());
        hash::HashRegisterString(off_trigger_name.c_str());

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
}

void game::RegisterGameComponents(mono::IEntityManager* entity_manager)
{
    entity_manager->RegisterComponent(PHYSICS_COMPONENT, CreatePhysics, ReleasePhysics, UpdatePhysics);
    entity_manager->RegisterComponent(CIRCLE_SHAPE_COMPONENT, CreateShape, ReleaseShape, UpdateCircleShape);
    entity_manager->RegisterComponent(BOX_SHAPE_COMPONENT, CreateShape, ReleaseShape, UpdateBoxShape);
    entity_manager->RegisterComponent(SEGMENT_SHAPE_COMPONENT, CreateShape, ReleaseShape, UpdateSegmentShape);
    entity_manager->RegisterComponent(POLYGON_SHAPE_COMPONENT, CreateShape, ReleaseShape, UpdatePolygonShape);
    entity_manager->RegisterComponent(HEALTH_COMPONENT, CreateHealth, ReleaseHealth, UpdateHealth);
    entity_manager->RegisterComponent(BEHAVIOUR_COMPONENT, CreateEntityLogic, ReleaseEntityLogic, UpdateEntityLogic);
    entity_manager->RegisterComponent(SPAWN_POINT_COMPONENT, CreateSpawnPoint, ReleaseSpawnPoint, UpdateSpawnPoint);
    entity_manager->RegisterComponent(ENTITY_SPAWN_POINT_COMPONENT, CreateEntitySpawnPoint, ReleaseEntitySpawnPoint, UpdateEntitySpawnPoint);
    entity_manager->RegisterComponent(SHAPE_TRIGGER_COMPONENT, CreateShapeTrigger, ReleaseShapeTrigger, UpdateShapeTrigger);
    entity_manager->RegisterComponent(DESTROYED_TRIGGER_COMPONENT, CreateDestroyedTrigger, ReleaseDestroyedTrigger, UpdateDestroyedTrigger);
    entity_manager->RegisterComponent(AREA_TRIGGER_COMPONENT, CreateAreaTrigger, ReleaseAreaTrigger, UpdateAreaTrigger);
    entity_manager->RegisterComponent(TIME_TRIGGER_COMPONENT, CreateTimeTrigger, ReleaseTimeTrigger, UpdateTimeTrigger);
    entity_manager->RegisterComponent(COUNTER_TRIGGER_COMPONENT, CreateCounterTrigger, ReleaseCounterTrigger, UpdateCounterTrigger);
    entity_manager->RegisterComponent(PICKUP_COMPONENT, CreatePickup, ReleasePickup, UpdatePickup);
    entity_manager->RegisterComponent(ANIMATION_COMPONENT, CreateAnimation, ReleaseAnimation, UpdateAnimation);
    entity_manager->RegisterComponent(TRANSLATION_COMPONENT, CreateTranslation, ReleaseTranslation, UpdateTranslation);
    entity_manager->RegisterComponent(ROTATION_COMPONENT, CreateRotation, ReleaseRotation, UpdateRotation);
    entity_manager->RegisterComponent(CAMERA_ZOOM_COMPONENT, CreateCameraZoom, ReleaseCameraZoom, UpdateCameraZoom);
    entity_manager->RegisterComponent(CAMERA_POINT_COMPONENT, CreateCameraPoint, ReleaseCameraPoint, UpdateCameraPoint);
    entity_manager->RegisterComponent(CAMERA_RESTORE_COMPONENT, CreateCameraRestore, ReleaseCameraRestore, UpdateCameraRestore);
    entity_manager->RegisterComponent(INTERACTION_COMPONENT, CreateInteraction, ReleaseInteraction, UpdateInteraction);
    entity_manager->RegisterComponent(INTERACTION_SWITCH_COMPONENT, CreateInteraction, ReleaseInteraction, UpdateInteractionSwitch);
    entity_manager->RegisterComponent(DIALOG_COMPONENT, CreateDialog, ReleaseDialog, UpdateDialog);
}
