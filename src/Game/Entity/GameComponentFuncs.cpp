
#include "GameComponentFuncs.h"

#include "SystemContext.h"
#include "TransformSystem/TransformSystem.h"
#include "Factories.h"

#include "DamageSystem.h"
#include "Entity/EntityManager.h"
#include "Entity/EntityLogicSystem.h"
#include "Entity/EntityLogicFactory.h"

#include "Physics/PhysicsSystem.h"

#include "Component.h"
#include "CollisionConfiguration.h"

#include "System/System.h"

namespace
{
    bool CreatePhysics(mono::Entity& entity, mono::SystemContext* context)
    {
        mono::BodyComponent body_params;
        body_params.mass = 1.0f;
        body_params.inertia = 1.0f;
        body_params.type = mono::BodyType::DYNAMIC;

        mono::PhysicsSystem* physics_system = context->GetSystem<mono::PhysicsSystem>();
        physics_system->AllocateBody(entity.id, body_params);
        return true;
    }

    bool ReleasePhysics(mono::Entity& entity, mono::SystemContext* context)
    {
        mono::PhysicsSystem* physics_system = context->GetSystem<mono::PhysicsSystem>();
        physics_system->ReleaseBody(entity.id);
        return true;
    }

    bool UpdatePhysics(mono::Entity& entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        mono::BodyComponent body_args;

        const bool found_type = FindAttribute(BODY_TYPE_ATTRIBUTE, properties, (int&)body_args.type);
        const bool found_mass = FindAttribute(MASS_ATTRIBUTE, properties, body_args.mass);
        const bool found_inertia = FindAttribute(INERTIA_ATTRIBUTE, properties, body_args.inertia);
        
        bool prevent_rotation = false;
        const bool found_prevent_rotation = FindAttribute(PREVENT_ROTATION_ATTRIBUTE, properties, prevent_rotation);

        const bool found_all = found_type && found_mass && found_inertia && found_prevent_rotation;
        if(!found_all)
        {
            System::Log("GameComponentFuncs|Missing physics parameters, will not update body for entity: %u\n", entity.id);
            if(!found_type)
                System::Log("GameComponentFuncs|Missing type\n");
            if(!found_mass)
                System::Log("GameComponentFuncs|Missing mass\n");
            if(!found_inertia)
                System::Log("GameComponentFuncs|Missing inertia\n");
            if(!found_prevent_rotation)
                System::Log("GameComponentFuncs|Missing prevent rotation\n");

            return false;
        }

        if(prevent_rotation)
            body_args.inertia = math::INF;

        mono::PhysicsSystem* physics_system = context->GetSystem<mono::PhysicsSystem>();
        mono::IBody* body = physics_system->GetBody(entity.id);
        body->SetMass(body_args.mass);
        body->SetMoment(body_args.inertia);
        body->SetType(body_args.type);

        mono::TransformSystem* transform_system = context->GetSystem<mono::TransformSystem>();
        const math::Matrix& transform = transform_system->GetTransform(entity.id);
        body->SetPosition(math::GetPosition(transform));
        return true;
    }

    bool CreateShape(mono::Entity& entity, mono::SystemContext* context)
    {
        return true;
    }

    bool ReleaseShape(mono::Entity& entity, mono::SystemContext* context)
    {
        return true;
    }

    bool UpdateCircleShape(mono::Entity& entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        int faction = 0;
        mono::CircleComponent shape_params;

        const bool found_faction = FindAttribute(FACTION_ATTRIBUTE, properties, faction);
        const bool found_radius = FindAttribute(RADIUS_ATTRIBUTE, properties, shape_params.radius);
        const bool found_position = FindAttribute(POSITION_ATTRIBUTE, properties, shape_params.offset);

        const bool found_all = found_faction && found_radius && found_position;
        if(!found_all)
        {
            System::Log("GameComponentFuncs|Unable to find paramenters for circle shape, entity: %u\n", entity.id);
            if(!found_faction)
                System::Log("GameComponentFuncs|Faction missing\n");
            if(!found_radius)
                System::Log("GameComponentFuncs|Radius missing\n");
            if(!found_position)
                System::Log("GameComponentFuncs|Position missing\n");

            return false;
        }

        const game::FactionPair& faction_pair = game::faction_lookup_table[faction];
        shape_params.category = faction_pair.category;
        shape_params.mask = faction_pair.mask;

        mono::PhysicsSystem* physics_system = context->GetSystem<mono::PhysicsSystem>();
        physics_system->AddShape(entity.id, shape_params);
        return true;
    }

    bool UpdateBoxShape(mono::Entity& entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        int faction;
        mono::BoxComponent shape_params;

        const bool found_faction = FindAttribute(FACTION_ATTRIBUTE, properties, faction);
        const bool found_width = FindAttribute(WIDTH_ATTRIBUTE, properties, shape_params.width);
        const bool found_height = FindAttribute(HEIGHT_ATTRIBUTE, properties, shape_params.height);
        const bool found_position = FindAttribute(POSITION_ATTRIBUTE, properties, shape_params.offset);

        const bool found_all = found_faction && found_width && found_height && found_position;
        if(!found_all)
        {
            System::Log("GameComponentFuncs|Unable to find paramenters for box shape, entity: %u\n", entity.id);
            if(!found_faction)
                System::Log("GameComponentFuncs|Faction missing\n");
            if(!found_width)
                System::Log("GameComponentFuncs|Width missing\n");
            if(!found_height)
                System::Log("GameComponentFuncs|Height missing\n");
            if(!found_position)
                System::Log("GameComponentFuncs|Position missing\n");

            return false;
        }

        const game::FactionPair& faction_pair = game::faction_lookup_table[faction];
        shape_params.category = faction_pair.category;
        shape_params.mask = faction_pair.mask;

        mono::PhysicsSystem* physics_system = context->GetSystem<mono::PhysicsSystem>();
        physics_system->AddShape(entity.id, shape_params);
        return true;
    }

    bool UpdateSegmentShape(mono::Entity& entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        int faction;
        mono::SegmentComponent shape_params;

        const bool found_faction = FindAttribute(FACTION_ATTRIBUTE, properties, faction);
        const bool found_radius = FindAttribute(RADIUS_ATTRIBUTE, properties, shape_params.radius);
        const bool found_start = FindAttribute(START_ATTRIBUTE, properties, shape_params.start);
        const bool found_end = FindAttribute(END_ATTRIBUTE, properties, shape_params.end);

        const bool found_all = found_faction && found_radius && found_start && found_end;
        if(!found_all)
        {
            System::Log("GameComponentFuncs|Unable to find paramenters for segment shape, entity: %u\n", entity.id);
            if(!found_faction)
                System::Log("GameComponentFuncs|Faction missing\n");
            if(!found_radius)
                System::Log("GameComponentFuncs|Radius missing\n");
            if(!found_start)
                System::Log("GameComponentFuncs|Start missing\n");
            if(!found_end)
                System::Log("GameComponentFuncs|End missing\n");

            return false;
        }

        const game::FactionPair& faction_pair = game::faction_lookup_table[faction];
        shape_params.category = faction_pair.category;
        shape_params.mask = faction_pair.mask;

        mono::PhysicsSystem* physics_system = context->GetSystem<mono::PhysicsSystem>();
        physics_system->AddShape(entity.id, shape_params);
        return true;
    }

    bool CreateHealth(mono::Entity& entity, mono::SystemContext* context)
    {
        game::DamageSystem* damage_system = context->GetSystem<game::DamageSystem>();
        damage_system->CreateRecord(entity.id);
        return true;
    }

    bool ReleaseHealth(mono::Entity& entity, mono::SystemContext* context)
    {
        game::DamageSystem* damage_system = context->GetSystem<game::DamageSystem>();
        damage_system->ReleaseRecord(entity.id);
        return true;
    }

    bool UpdateHealth(mono::Entity& entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        int health;
        if(!FindAttribute(HEALTH_ATTRIBUTE, properties, health))
        {
            System::Log("GameComponentFuncs|Missing health parameters, will not update component.\n");
            return false;
        }

        game::DamageSystem* damage_system = context->GetSystem<game::DamageSystem>();
        game::DamageRecord* damage_record = damage_system->GetDamageRecord(entity.id);
        damage_record->health = health;
        damage_record->full_health = health;

        return true;
    }

    bool CreateEntityLogic(mono::Entity& entity, mono::SystemContext* context)
    {
        game::EntityLogicSystem* logic_system = context->GetSystem<game::EntityLogicSystem>();
        (void)logic_system;
        return true;
    }

    bool ReleaseEntityLogic(mono::Entity& entity, mono::SystemContext* context)
    {
        game::EntityLogicSystem* logic_system = context->GetSystem<game::EntityLogicSystem>();
        logic_system->ReleaseLogic(entity.id);
        return true;
    }

    bool UpdateEntityLogic(mono::Entity& entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
    {
        int logic_type_value;
        const bool found_property = FindAttribute(ENTITY_BEHAVIOUR_ATTRIBUTE, properties, logic_type_value);
        if(!found_property)
            return false;

        game::IEntityLogic* entity_logic = nullptr;

        const EntityLogicType logic_type = EntityLogicType(logic_type_value);
        if(logic_type == EntityLogicType::INVADER_PATH)
        {
            const char* path_file = nullptr;
            const bool found_path_property = FindAttribute(PATH_FILE_ATTRIBUTE, properties, path_file);
            if(!found_path_property)
                return false;

            entity_logic = game::g_logic_factory->CreatePathInvaderLogic(path_file, entity.id);
        }
        else
        {
            entity_logic = game::g_logic_factory->CreateLogic(logic_type, entity.id);
        }

        game::EntityLogicSystem* logic_system = context->GetSystem<game::EntityLogicSystem>();
        logic_system->AddLogic(entity.id, entity_logic);

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
}
