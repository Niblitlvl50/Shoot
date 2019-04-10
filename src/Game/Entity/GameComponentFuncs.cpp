
#include "GameComponentFuncs.h"

#include "SystemContext.h"
#include "EntitySystem.h"
#include "TransformSystem.h"
#include "Factories.h"

#include "DamageSystem.h"
#include "Entity/EntityManager.h"
#include "Entity/EntityLogicSystem.h"
#include "Entity/EntityLogicFactory.h"

#include "Physics/PhysicsSystem.h"

#include "Component.h"
#include "CollisionConfiguration.h"

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

        const bool found_type = world::FindAttribute(world::BODY_TYPE_ATTRIBUTE, properties, (int&)body_args.type);
        const bool found_mass = world::FindAttribute(world::MASS_ATTRIBUTE, properties, body_args.mass);
        const bool found_inertia = world::FindAttribute(world::INERTIA_ATTRIBUTE, properties, body_args.inertia);

        const bool found_all = found_type && found_mass && found_inertia;
        if(!found_all)
        {
            std::printf("Missing physics parameters, will not update body for entity: %u\n", entity.id);
            if(!found_type)
                std::printf("Missing type\n");
            if(!found_mass)
                std::printf("Missing mass\n");
            if(!found_inertia)
                std::printf("Missing inertia\n");

            return false;
        }

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

        const bool found_faction = world::FindAttribute(world::FACTION_ATTRIBUTE, properties, faction);
        const bool found_radius = world::FindAttribute(world::RADIUS_ATTRIBUTE, properties, shape_params.radius);
        const bool found_position = world::FindAttribute(world::POSITION_ATTRIBUTE, properties, shape_params.offset);

        const bool found_all = found_faction && found_radius && found_position;
        if(!found_all)
        {
            std::printf("Unable to find paramenters for circle shape, entity: %u\n", entity.id);
            if(!found_faction)
                std::printf("Faction missing\n");
            if(!found_radius)
                std::printf("Radius missing\n");
            if(!found_position)
                std::printf("Position missing\n");

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

        const bool found_faction = world::FindAttribute(world::FACTION_ATTRIBUTE, properties, faction);
        const bool found_width = world::FindAttribute(world::WIDTH_ATTRIBUTE, properties, shape_params.width);
        const bool found_height = world::FindAttribute(world::HEIGHT_ATTRIBUTE, properties, shape_params.height);
        const bool found_position = world::FindAttribute(world::POSITION_ATTRIBUTE, properties, shape_params.offset);

        const bool found_all = found_faction && found_width && found_height && found_position;
        if(!found_all)
        {
            std::printf("Unable to find paramenters for box shape, entity: %u\n", entity.id);
            if(!found_faction)
                std::printf("Faction missing\n");
            if(!found_width)
                std::printf("Width missing\n");
            if(!found_height)
                std::printf("Height missing\n");
            if(!found_position)
                std::printf("Position missing\n");

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

        const bool found_faction = world::FindAttribute(world::FACTION_ATTRIBUTE, properties, faction);
        const bool found_radius = world::FindAttribute(world::RADIUS_ATTRIBUTE, properties, shape_params.radius);
        const bool found_start = world::FindAttribute(world::START_ATTRIBUTE, properties, shape_params.start);
        const bool found_end = world::FindAttribute(world::END_ATTRIBUTE, properties, shape_params.end);

        const bool found_all = found_faction && found_radius && found_start && found_end;
        if(!found_all)
        {
            std::printf("Unable to find paramenters for segment shape, entity: %u\n", entity.id);
            if(!found_faction)
                std::printf("Faction missing\n");
            if(!found_radius)
                std::printf("Radius missing\n");
            if(!found_start)
                std::printf("Start missing\n");
            if(!found_end)
                std::printf("End missing\n");

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
        if(!world::FindAttribute(world::HEALTH_ATTRIBUTE, properties, health))
        {
            std::printf("Missing health parameters, will not update component.\n");
            return false;
        }

        game::DamageSystem* damage_system = context->GetSystem<game::DamageSystem>();
        game::DamageRecord* damage_record = damage_system->GetDamageRecord(entity.id);
        damage_record->health = health;

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
        const bool found_property = world::FindAttribute(world::ENTITY_BEHAVIOUR_ATTRIBUTE, properties, logic_type_value);
        if(!found_property)
            return false;

        game::IEntityLogic* entity_logic = nullptr;

        const EntityLogicType logic_type = EntityLogicType(logic_type_value);
        if(logic_type == EntityLogicType::INVADER_PATH)
        {
            const char* path_file = nullptr;
            const bool found_path_property = world::FindAttribute(world::PATH_FILE_ATTRIBUTE, properties, path_file);
            if(!found_path_property)
                return false;

            entity_logic = game::logic_factory->CreatePathInvaderLogic(path_file, entity.id);
        }
        else
        {
            entity_logic = game::logic_factory->CreateLogic(logic_type, entity.id);
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
