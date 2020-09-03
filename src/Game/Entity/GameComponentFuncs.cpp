
#include "GameComponentFuncs.h"

#include "SystemContext.h"
#include "TransformSystem/TransformSystem.h"
#include "Factories.h"

#include "DamageSystem.h"
#include "TriggerSystem.h"
#include "Pickups/PickupSystem.h"
#include "SpawnSystem.h"
#include "Entity/EntityManager.h"
#include "Entity/EntityLogicSystem.h"
#include "Entity/EntityLogicFactory.h"
#include "Entity/ModificationSystem.h"

#include "Physics/PhysicsSystem.h"
#include "Pickups/PickupSystem.h"

#include "Component.h"
#include "CollisionConfiguration.h"

#include "Util/Hash.h"
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

        if(prevent_rotation)
            body_args.inertia = math::INF;

        mono::PhysicsSystem* physics_system = context->GetSystem<mono::PhysicsSystem>();
        mono::IBody* body = physics_system->GetBody(entity->id);
        body->SetMass(body_args.mass);
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

        int score;
        FindAttribute(SCORE_ATTRIBUTE, properties, score, FallbackMode::SET_DEFAULT);

        bool is_boss_health;
        FindAttribute(BOSS_HEALTH_ATTRIBUTE, properties, is_boss_health, FallbackMode::SET_DEFAULT);

        game::DamageSystem* damage_system = context->GetSystem<game::DamageSystem>();
        game::DamageRecord* damage_record = damage_system->GetDamageRecord(entity->id);
        damage_record->health = health;
        damage_record->full_health = health;
        damage_record->score = score;
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

        game::IEntityLogic* entity_logic = nullptr;

        const shared::EntityLogicType logic_type = shared::EntityLogicType(logic_type_value);
        if(logic_type == shared::EntityLogicType::INVADER_PATH)
        {
            const char* path_file = nullptr;
            const bool found_path_property = FindAttribute(PATH_FILE_ATTRIBUTE, properties, path_file, FallbackMode::REQUIRE_ATTRIBUTE);
            if(!found_path_property || strlen(path_file) == 0)
                return false;

            entity_logic = game::g_logic_factory->CreatePathInvaderLogic(path_file, entity->id);
        }
        else
        {
            entity_logic = game::g_logic_factory->CreateLogic(logic_type, entity->id);
        }

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
        game::SpawnSystem::SpawnPoint spawn_point;
        FindAttribute(SPAWN_SCORE_ATTRIBUTE, properties, spawn_point.spawn_score, FallbackMode::SET_DEFAULT);

        game::SpawnSystem* spawn_system = context->GetSystem<game::SpawnSystem>();
        spawn_system->SetSpawnPointData(entity->id, spawn_point);
        return true;
    }

    bool CreateShapeTrigger(mono::Entity* entity, mono::SystemContext* context)
    {
        game::TriggerSystem* trigger_system = context->GetSystem<game::TriggerSystem>();
        trigger_system->AllocateTrigger(entity->id);
        return true;
    }
    
    bool ReleaseShapeTrigger(mono::Entity* entity, mono::SystemContext* context)
    {
        return true;
    }
    
    bool UpdateShapeTrigger(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        const char* trigger_name = nullptr;
        const bool found_trigger_name =
            FindAttribute(TRIGGER_NAME_ATTRIBUTE, properties, trigger_name, FallbackMode::REQUIRE_ATTRIBUTE);

        if(!found_trigger_name)
        {
            System::Log("GameComponentFunctions|Missing trigger name parameter, unable to update component\n");
            return false;
        }

        uint32_t faction;
        FindAttribute(FACTION_PICKER_ATTRIBUTE, properties, faction, FallbackMode::SET_DEFAULT);

        game::TriggerSystem* trigger_system = context->GetSystem<game::TriggerSystem>();
        trigger_system->AddShapeTrigger(entity->id, mono::Hash(trigger_name), faction);
        return true;
    }

    bool CreateDeathTrigger(mono::Entity* entity, mono::SystemContext* context)
    {
        return true;
    }

    bool ReleaseDeathTrigger(mono::Entity* entity, mono::SystemContext* context)
    {
        return true;
    }

    bool UpdateDeathTrigger(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        const char* trigger_name = nullptr;
        const bool found_trigger_name =
            FindAttribute(TRIGGER_NAME_ATTRIBUTE, properties, trigger_name, FallbackMode::REQUIRE_ATTRIBUTE);

        if(!found_trigger_name)
        {
            System::Log("GameComponentFunctions|Missing trigger name parameter, unable to update component\n");
            return false;
        }

        game::TriggerSystem* trigger_system = context->GetSystem<game::TriggerSystem>();
        trigger_system->AddDeathTrigger(entity->id, mono::Hash(trigger_name));
        return true;
    }

    bool CreateAreaTrigger(mono::Entity* entity, mono::SystemContext* context)
    {
        return true;
    }

    bool ReleaseAreaTrigger(mono::Entity* entity, mono::SystemContext* context)
    {
        return true;
    }

    bool UpdateAreaTrigger(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        const char* trigger_name = nullptr;
        const bool found_trigger_name =
            FindAttribute(TRIGGER_NAME_ATTRIBUTE, properties, trigger_name, FallbackMode::REQUIRE_ATTRIBUTE);
        if(!found_trigger_name)
            return false;

        math::Vector size;
        uint32_t faction;

        FindAttribute(SIZE_ATTRIBUTE, properties, size, FallbackMode::SET_DEFAULT);
        FindAttribute(FACTION_PICKER_ATTRIBUTE, properties, faction, FallbackMode::SET_DEFAULT);

        mono::TransformSystem* transform_system = context->GetSystem<mono::TransformSystem>();
        const math::Matrix& world_transform = transform_system->GetWorld(entity->id);

        const math::Quad world_bb = math::Transform(world_transform, math::Quad(0.0f, 0.0f, size.x, size.y));

        game::TriggerSystem* trigger_system = context->GetSystem<game::TriggerSystem>();
        trigger_system->AddAreaEntityTrigger(entity->id, mono::Hash(trigger_name), world_bb, faction, 0);

        return true;
    }

    bool CreateTimeTrigger(mono::Entity* entity, mono::SystemContext* context)
    {
        return true;
    }

    bool ReleaseTimeTrigger(mono::Entity* entity, mono::SystemContext* context)
    {
        return true;
    }

    bool UpdateTimeTrigger(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        const char* trigger_name = nullptr;
        const bool found_trigger_name =
            FindAttribute(TRIGGER_NAME_ATTRIBUTE, properties, trigger_name, FallbackMode::REQUIRE_ATTRIBUTE);
        if(!found_trigger_name)
            return false;

        int timeout_ms;
        FindAttribute(TIME_STAMP_ATTRIBUTE, properties, timeout_ms, FallbackMode::SET_DEFAULT);

        game::TriggerSystem* trigger_system = context->GetSystem<game::TriggerSystem>();
        trigger_system->AddTimeTrigger(entity->id, mono::Hash(trigger_name), timeout_ms);

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
        FindAttribute(PICKUP_TYPE_ATTRIBUTE, properties, (uint32_t&)pickup.type, FallbackMode::SET_DEFAULT);
        FindAttribute(AMOUNT_ATTRIBUTE, properties, pickup.amount, FallbackMode::SET_DEFAULT);

        game::PickupSystem* pickup_system = context->GetSystem<game::PickupSystem>();
        pickup_system->SetPickupData(entity->id, pickup);

        return true;
    }

    bool CreateAnimation(mono::Entity* entity, mono::SystemContext* context)
    {
        return true;
    }

    bool ReleaseAnimation(mono::Entity* entity, mono::SystemContext* context)
    {
        return true;
    }

    bool UpdateAnimation(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        const char* trigger_name = nullptr;
        const bool found_trigger_name =
            FindAttribute(TRIGGER_NAME_ATTRIBUTE, properties, trigger_name, FallbackMode::REQUIRE_ATTRIBUTE);

        if(!found_trigger_name)
        {
            System::Log("GameComponentFunctions|Missing trigger name parameter, unable to update component\n");
            return false;
        }

        uint32_t animation_index;
        FindAttribute(ANIMATION_ATTRIBUTE, properties, animation_index, FallbackMode::SET_DEFAULT);

        game::ModificationSystem* modification_system = context->GetSystem<game::ModificationSystem>();
        modification_system->AddAnimationComponent(entity->id, mono::Hash(trigger_name), animation_index);

        return true;
    }

    bool CreateTranslation(mono::Entity* entity, mono::SystemContext* context)
    {
        return true;
    }

    bool ReleaseTranslation(mono::Entity* entity, mono::SystemContext* context)
    {
        return true;
    }

    bool UpdateTranslation(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        const char* trigger_name = nullptr;
        const bool found_trigger_name =
            FindAttribute(TRIGGER_NAME_ATTRIBUTE, properties, trigger_name, FallbackMode::REQUIRE_ATTRIBUTE);

        if(!found_trigger_name)
        {
            System::Log("GameComponentFunctions|Missing trigger name parameter, unable to update component\n");
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

        game::ModificationSystem* modification_system = context->GetSystem<game::ModificationSystem>();
        modification_system->AddTranslationComponent(
            entity->id, mono::Hash(trigger_name), duration, math::ease_functions[ease_func_index], shared::AnimationMode(animation_mode), translation);

        return true;
    }

    bool CreateRotation(mono::Entity* entity, mono::SystemContext* context)
    {
        return true;
    }

    bool ReleaseRotation(mono::Entity* entity, mono::SystemContext* context)
    {
        return true;
    }

    bool UpdateRotation(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        const char* trigger_name = nullptr;
        const bool found_trigger_name =
            FindAttribute(TRIGGER_NAME_ATTRIBUTE, properties, trigger_name, FallbackMode::REQUIRE_ATTRIBUTE);

        if(!found_trigger_name)
        {
            System::Log("GameComponentFunctions|Missing trigger name parameter, unable to update component\n");
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

        game::ModificationSystem* modification_system = context->GetSystem<game::ModificationSystem>();
        modification_system->AddRotationComponent(
            entity->id, mono::Hash(trigger_name), duration, math::ease_functions[ease_func_index], shared::AnimationMode(animation_mode), rotation);

        return true;
    }
}

void game::RegisterGameComponents(EntityManager& entity_manager)
{
    entity_manager.RegisterComponent(PHYSICS_COMPONENT, CreatePhysics, ReleasePhysics, UpdatePhysics);
    entity_manager.RegisterComponent(CIRCLE_SHAPE_COMPONENT, CreateShape, ReleaseShape, UpdateCircleShape);
    entity_manager.RegisterComponent(BOX_SHAPE_COMPONENT, CreateShape, ReleaseShape, UpdateBoxShape);
    entity_manager.RegisterComponent(SEGMENT_SHAPE_COMPONENT, CreateShape, ReleaseShape, UpdateSegmentShape);
    entity_manager.RegisterComponent(HEALTH_COMPONENT, CreateHealth, ReleaseHealth, UpdateHealth);
    entity_manager.RegisterComponent(BEHAVIOUR_COMPONENT, CreateEntityLogic, ReleaseEntityLogic, UpdateEntityLogic);
    entity_manager.RegisterComponent(SPAWN_POINT_COMPONENT, CreateSpawnPoint, ReleaseSpawnPoint, UpdateSpawnPoint);
    entity_manager.RegisterComponent(SHAPE_TRIGGER_COMPONENT, CreateShapeTrigger, ReleaseShapeTrigger, UpdateShapeTrigger);
    entity_manager.RegisterComponent(DEATH_TRIGGER_COMPONENT, CreateDeathTrigger, ReleaseDeathTrigger, UpdateDeathTrigger);
    entity_manager.RegisterComponent(AREA_TRIGGER_COMPONENT, CreateAreaTrigger, ReleaseAreaTrigger, UpdateAreaTrigger);
    entity_manager.RegisterComponent(TIME_TRIGGER_COMPONENT, CreateTimeTrigger, ReleaseTimeTrigger, UpdateTimeTrigger);
    entity_manager.RegisterComponent(PICKUP_COMPONENT, CreatePickup, ReleasePickup, UpdatePickup);
    entity_manager.RegisterComponent(ANIMATION_COMPONENT, CreateAnimation, ReleaseAnimation, UpdateAnimation);
    entity_manager.RegisterComponent(TRANSLATION_COMPONENT, CreateTranslation, ReleaseTranslation, UpdateTranslation);
    entity_manager.RegisterComponent(ROTATION_COMPONENT, CreateRotation, ReleaseRotation, UpdateRotation);
}
