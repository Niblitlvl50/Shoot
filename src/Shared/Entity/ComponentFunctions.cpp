
#include "ComponentFunctions.h"
#include "System/System.h"

#include "SystemContext.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "TransformSystem/TransformSystem.h"

#include "Entity/EntityManager.h"

#include "Component.h"

bool CreateTransform(mono::Entity& entity, mono::SystemContext* context)
{
    // No need to do anything, transform system is just a passive system.
    return true;
}

bool ReleaseTransform(mono::Entity& entity, mono::SystemContext* context)
{
    mono::TransformSystem* transform_system = context->GetSystem<mono::TransformSystem>();
    transform_system->UnchildTransform(entity.id);

    return true;
}

bool UpdateTransform(mono::Entity& entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
{
    math::Vector position;
    float rotation = 0.0f;
    FindAttribute(POSITION_ATTRIBUTE, properties, position, FallbackMode::SET_DEFAULT);
    FindAttribute(ROTATION_ATTRIBUTE, properties, rotation, FallbackMode::SET_DEFAULT);

    mono::TransformSystem* transform_system = context->GetSystem<mono::TransformSystem>();
    math::Matrix& transform = transform_system->GetTransform(entity.id);
    transform = math::CreateMatrixFromZRotation(rotation);
    math::Position(transform, position);

    return true;
}

std::vector<Attribute> GetTransform(const mono::Entity& entity, mono::SystemContext* context)
{
    mono::TransformSystem* transform_system = context->GetSystem<mono::TransformSystem>();
    const math::Matrix& transform = transform_system->GetTransform(entity.id);

    return {
       { POSITION_ATTRIBUTE, math::GetPosition(transform) },
       { ROTATION_ATTRIBUTE, math::GetZRotation(transform) }
    };
}

bool CreateSprite(mono::Entity& entity, mono::SystemContext* context)
{
    mono::SpriteSystem* sprite_system = context->GetSystem<mono::SpriteSystem>();
    sprite_system->AllocateSprite(entity.id);
    return true;
}

bool ReleaseSprite(mono::Entity& entity, mono::SystemContext* context)
{
    mono::SpriteSystem* sprite_system = context->GetSystem<mono::SpriteSystem>();
    sprite_system->ReleaseSprite(entity.id);
    return true;
}

bool UpdateSprite(mono::Entity& entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
{
    mono::SpriteComponents sprite_args;

    const bool success = FindAttribute(SPRITE_ATTRIBUTE, properties, sprite_args.sprite_file, FallbackMode::REQUIRE_ATTRIBUTE);
    if(!success)
    {
        System::Log("ComponentFunctions|Missing sprite parameters, unable to update component\n");
        return false;
    }
    
    FindAttribute(COLOR_ATTRIBUTE, properties, sprite_args.shade, FallbackMode::SET_DEFAULT);
    FindAttribute(ANIMATION_ATTRIBUTE, properties, sprite_args.animation_id, FallbackMode::SET_DEFAULT);
    FindAttribute(FLIP_VERTICAL_ATTRIBUTE, properties, sprite_args.flip_vertical, FallbackMode::SET_DEFAULT);
    FindAttribute(FLIP_HORIZONTAL_ATTRIBUTE, properties, sprite_args.flip_horizontal, FallbackMode::SET_DEFAULT);

    char sprite_file[1024] = { 0 };
    std::sprintf(sprite_file, "res/sprites/%s", sprite_args.sprite_file);
    sprite_args.sprite_file = sprite_file;

    mono::SpriteSystem* sprite_system = context->GetSystem<mono::SpriteSystem>();
    sprite_system->SetSpriteData(entity.id, sprite_args);
    return success;
}
 
void shared::RegisterSharedComponents(EntityManager& entity_manager)
{
    entity_manager.RegisterComponent(TRANSFORM_COMPONENT, CreateTransform, ReleaseTransform, UpdateTransform, GetTransform);
    entity_manager.RegisterComponent(SPRITE_COMPONENT, CreateSprite, ReleaseSprite, UpdateSprite);
}
