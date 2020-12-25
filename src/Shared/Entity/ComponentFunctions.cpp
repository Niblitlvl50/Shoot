
#include "ComponentFunctions.h"
#include "System/System.h"

#include "SystemContext.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Text/TextSystem.h"
#include "TransformSystem/TransformSystem.h"

#include "EntitySystem/EntityManager.h"

#include "Component.h"

bool CreateTransform(mono::Entity* entity, mono::SystemContext* context)
{
    // No need to do anything, transform system is just a passive system.
    return true;
}

bool ReleaseTransform(mono::Entity* entity, mono::SystemContext* context)
{
    mono::TransformSystem* transform_system = context->GetSystem<mono::TransformSystem>();
    transform_system->ResetTransformComponent(entity->id);
    return true;
}

bool UpdateTransform(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
{
    math::Vector position;
    float rotation = 0.0f;
    FindAttribute(POSITION_ATTRIBUTE, properties, position, FallbackMode::SET_DEFAULT);
    FindAttribute(ROTATION_ATTRIBUTE, properties, rotation, FallbackMode::SET_DEFAULT);

    mono::TransformSystem* transform_system = context->GetSystem<mono::TransformSystem>();
    math::Matrix& transform = transform_system->GetTransform(entity->id);
    transform = math::CreateMatrixFromZRotation(rotation);
    math::Position(transform, position);

    return true;
}

std::vector<Attribute> GetTransform(const mono::Entity* entity, mono::SystemContext* context)
{
    mono::TransformSystem* transform_system = context->GetSystem<mono::TransformSystem>();
    const math::Matrix& transform = transform_system->GetTransform(entity->id);

    return {
       { POSITION_ATTRIBUTE, Variant(math::GetPosition(transform)) },
       { ROTATION_ATTRIBUTE, Variant(math::GetZRotation(transform)) }
    };
}

bool CreateSprite(mono::Entity* entity, mono::SystemContext* context)
{
    mono::SpriteSystem* sprite_system = context->GetSystem<mono::SpriteSystem>();
    sprite_system->AllocateSprite(entity->id);
    return true;
}

bool ReleaseSprite(mono::Entity* entity, mono::SystemContext* context)
{
    mono::SpriteSystem* sprite_system = context->GetSystem<mono::SpriteSystem>();
    sprite_system->ReleaseSprite(entity->id);
    return true;
}

bool UpdateSprite(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
{
    std::string sprite_file;
    mono::SpriteComponents sprite_args;

    const bool success = FindAttribute(SPRITE_ATTRIBUTE, properties, sprite_file, FallbackMode::REQUIRE_ATTRIBUTE);
    if(!success)
    {
        System::Log("ComponentFunctions|Missing sprite parameters, unable to update component\n");
        return false;
    }
    
    FindAttribute(COLOR_ATTRIBUTE, properties, sprite_args.shade, FallbackMode::SET_DEFAULT);
    FindAttribute(ANIMATION_ATTRIBUTE, properties, sprite_args.animation_id, FallbackMode::SET_DEFAULT);
    FindAttribute(SPRITE_PROPERTIES_ATTRIBUTE, properties, sprite_args.properties, FallbackMode::SET_DEFAULT);
    FindAttribute(SHADOW_OFFSET_ATTRIBUTE, properties, sprite_args.shadow_offset, FallbackMode::SET_DEFAULT);
    FindAttribute(SHADOW_SIZE_ATTRIBUTE, properties, sprite_args.shadow_size, FallbackMode::SET_DEFAULT);
    FindAttribute(FLIP_VERTICAL_ATTRIBUTE, properties, sprite_args.flip_vertical, FallbackMode::SET_DEFAULT);
    FindAttribute(FLIP_HORIZONTAL_ATTRIBUTE, properties, sprite_args.flip_horizontal, FallbackMode::SET_DEFAULT);
    FindAttribute(SPRITE_LAYER_ATTRIBUTE, properties, sprite_args.layer, FallbackMode::SET_DEFAULT);
    FindAttribute(RANDOM_START_FRAME_ATTRIBUTE, properties, sprite_args.random_start_frame, FallbackMode::SET_DEFAULT);

    char sprite_path[1024] = { 0 };
    std::sprintf(sprite_path, "res/sprites/%s", sprite_file.c_str());
    sprite_args.sprite_file = sprite_path;

    mono::SpriteSystem* sprite_system = context->GetSystem<mono::SpriteSystem>();
    sprite_system->SetSpriteData(entity->id, sprite_args);
    return success;
}

bool CreateText(mono::Entity* entity, mono::SystemContext* context)
{
    mono::TextSystem* text_system = context->GetSystem<mono::TextSystem>();
    text_system->AllocateText(entity->id);
    return true;
}

bool ReleaseText(mono::Entity* entity, mono::SystemContext* context)
{
    mono::TextSystem* text_system = context->GetSystem<mono::TextSystem>();
    text_system->ReleaseText(entity->id);
    return true;
}

bool UpdateText(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
{
    mono::TextComponent text_component;

    FindAttribute(TEXT_ATTRIBUTE, properties, text_component.text, FallbackMode::SET_DEFAULT);
    FindAttribute(FONT_ID_ATTRIBUTE, properties, text_component.font_id, FallbackMode::SET_DEFAULT);
    FindAttribute(COLOR_ATTRIBUTE, properties, text_component.tint, FallbackMode::SET_DEFAULT);
    FindAttribute(CENTER_ATTRIBUTE, properties, text_component.centered, FallbackMode::SET_DEFAULT);
    FindAttribute(TEXT_SHADOW_ATTRIBUTE, properties, text_component.draw_shadow, FallbackMode::SET_DEFAULT);

    mono::TextSystem* text_system = context->GetSystem<mono::TextSystem>();
    text_system->SetTextData(entity->id, text_component);

    return true;
}

void shared::RegisterSharedComponents(mono::EntityManager& entity_manager)
{
    entity_manager.RegisterComponent(TRANSFORM_COMPONENT, CreateTransform, ReleaseTransform, UpdateTransform, GetTransform);
    entity_manager.RegisterComponent(SPRITE_COMPONENT, CreateSprite, ReleaseSprite, UpdateSprite);
    entity_manager.RegisterComponent(TEXT_COMPONENT, CreateText, ReleaseText, UpdateText);
}
