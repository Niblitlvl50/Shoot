
#include "ComponentFunctions.h"
#include "System/System.h"

#include "SystemContext.h"
#include "Particle/ParticleSystem.h"
#include "Particle/ParticleFwd.h"
#include "Paths/PathSystem.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Text/TextSystem.h"
#include "Rendering/Lights/LightSystem.h"
#include "TransformSystem/TransformSystem.h"
#include "RoadSystem/RoadSystem.h"

#include "EntitySystem/IEntityManager.h"

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

    transform_system->SetTransformState(entity->id, mono::TransformState::CLIENT);

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
        System::Log("ComponentFunctions|Missing sprite parameters, unable to update component");
        return false;
    }
    
    FindAttribute(COLOR_ATTRIBUTE, properties, sprite_args.shade, FallbackMode::SET_DEFAULT);
    FindAttribute(ANIMATION_ATTRIBUTE, properties, sprite_args.animation_id, FallbackMode::SET_DEFAULT);
    FindAttribute(SPRITE_PROPERTIES_ATTRIBUTE, properties, sprite_args.properties, FallbackMode::SET_DEFAULT);
    FindAttribute(SHADOW_OFFSET_ATTRIBUTE, properties, sprite_args.shadow_offset, FallbackMode::SET_DEFAULT);
    FindAttribute(SHADOW_SIZE_ATTRIBUTE, properties, sprite_args.shadow_size, FallbackMode::SET_DEFAULT);
    FindAttribute(LAYER_ATTRIBUTE, properties, sprite_args.layer, FallbackMode::SET_DEFAULT);
    FindAttribute(SORT_OFFSET_ATTRIBUTE, properties, sprite_args.sort_offset, FallbackMode::SET_DEFAULT);
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
    FindAttribute(CENTER_FLAGS_ATTRIBUTE, properties, (uint32_t&)text_component.center_flags, FallbackMode::SET_DEFAULT);
    FindAttribute(TEXT_SHADOW_ATTRIBUTE, properties, text_component.draw_shadow, FallbackMode::SET_DEFAULT);

    mono::TextSystem* text_system = context->GetSystem<mono::TextSystem>();
    text_system->SetTextData(entity->id, text_component);

    return true;
}

bool CreatePath(mono::Entity* entity, mono::SystemContext* context)
{
    mono::PathSystem* path_system = context->GetSystem<mono::PathSystem>();
    path_system->AllocatePath(entity->id);
    return true;
}

bool ReleasePath(mono::Entity* entity, mono::SystemContext* context)
{
    mono::PathSystem* path_system = context->GetSystem<mono::PathSystem>();
    path_system->ReleasePath(entity->id);
    return true;
}

bool UpdatePath(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
{
    int path_type;
    FindAttribute(PATH_TYPE_ATTRIBUTE, properties, path_type, FallbackMode::SET_DEFAULT);

    mono::PathComponent component;
    component.type = mono::PathType(path_type);

    FindAttribute(PATH_POINTS_ATTRIBUTE, properties, component.points, FallbackMode::REQUIRE_ATTRIBUTE);
    FindAttribute(PATH_CLOSED_ATTRIBUTE, properties, component.closed, FallbackMode::SET_DEFAULT);

    mono::PathSystem* path_system = context->GetSystem<mono::PathSystem>();
    path_system->SetPathData(entity->id, component);

    return true;
}

bool CreateRoad(mono::Entity* entity, mono::SystemContext* context)
{
    mono::RoadSystem* road_system = context->GetSystem<mono::RoadSystem>();
    road_system->Allocate(entity->id);
    return true;
}

bool ReleaseRoad(mono::Entity* entity, mono::SystemContext* context)
{
    mono::RoadSystem* road_system = context->GetSystem<mono::RoadSystem>();
    road_system->Release(entity->id);
    return true;
}

bool UpdateRoad(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
{
    mono::RoadComponent component;
    FindAttribute(WIDTH_ATTRIBUTE, properties, component.width, FallbackMode::SET_DEFAULT);
    FindAttribute(TEXTURE_ATTRIBUTE, properties, component.texture_name, FallbackMode::SET_DEFAULT);

    mono::RoadSystem* road_system = context->GetSystem<mono::RoadSystem>();
    road_system->SetData(entity->id, component);

    return true;
}

bool CreateLight(mono::Entity* entity, mono::SystemContext* context)
{
    mono::LightSystem* light_system = context->GetSystem<mono::LightSystem>();
    light_system->Allocate(entity->id);
    return true;
}

bool ReleaseLight(mono::Entity* entity, mono::SystemContext* context)
{
    mono::LightSystem* light_system = context->GetSystem<mono::LightSystem>();
    light_system->Release(entity->id);
    return true;
}

bool UpdateLight(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
{
    mono::LightComponent component;
    FindAttribute(RADIUS_ATTRIBUTE, properties, component.radius, FallbackMode::SET_DEFAULT);
    FindAttribute(OFFSET_ATTRIBUTE, properties, component.offset, FallbackMode::SET_DEFAULT);
    FindAttribute(COLOR_ATTRIBUTE, properties, component.shade, FallbackMode::SET_DEFAULT);
    FindAttribute(FLICKER_ATTRIBUTE, properties, component.flicker, FallbackMode::SET_DEFAULT);
    FindAttribute(FREQUENCY_ATTRIBUTE, properties, component.flicker_frequencey, FallbackMode::SET_DEFAULT);
    FindAttribute(PERCENTAGE_ATTRIBUTE, properties, component.flicker_percentage, FallbackMode::SET_DEFAULT);

    mono::LightSystem* light_system = context->GetSystem<mono::LightSystem>();
    light_system->SetData(entity->id, component);

    return true;
}

bool CreateParticleSystem(mono::Entity* entity, mono::SystemContext* context)
{
    mono::ParticleSystem* particle_system = context->GetSystem<mono::ParticleSystem>();
    particle_system->AllocatePool(entity->id);
    return true;
}

bool ReleaseParticleSystem(mono::Entity* entity, mono::SystemContext* context)
{
    mono::ParticleSystem* particle_system = context->GetSystem<mono::ParticleSystem>();
    particle_system->ReleasePool(entity->id);
    return true;
}

bool UpdateParticleSystem(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
{
    int pool_size;
    std::string texture_file;
    int blend_mode;
    bool local_space = true;
    float damping;
    FindAttribute(POOL_SIZE_ATTRIBUTE, properties, pool_size, FallbackMode::SET_DEFAULT);
    FindAttribute(TEXTURE_ATTRIBUTE, properties, texture_file, FallbackMode::SET_DEFAULT);
    FindAttribute(BLEND_MODE_ATTRIBUTE, properties, blend_mode, FallbackMode::SET_DEFAULT);
    FindAttribute(DAMPING_ATTRIBUTE, properties, damping, FallbackMode::SET_DEFAULT);

    mono::ParticleSystem* particle_system = context->GetSystem<mono::ParticleSystem>();
    particle_system->SetPoolData(entity->id, pool_size, texture_file.c_str(), mono::BlendMode(blend_mode), mono::ParticleTransformSpace::LOCAL, damping, mono::DefaultUpdater);

    return true;
}

bool CreateBoxEmitter(mono::Entity* entity, mono::SystemContext* context)
{
    return true;
}

bool ReleaseBoxEmitter(mono::Entity* entity, mono::SystemContext* context)
{
    return true;
}

bool UpdateBoxEmitter(mono::Entity* entity, const std::vector<Attribute>& properties, mono::SystemContext* context)
{
    float duration;
    float emit_rate;
    int emitter_type;
    mono::ParticleGeneratorProperties generator_properties;

    FindAttribute(DURATION_ATTRIBUTE, properties, duration, FallbackMode::SET_DEFAULT);
    FindAttribute(EMIT_RATE_ATTRIBUTE, properties, emit_rate, FallbackMode::SET_DEFAULT);
    FindAttribute(EMITTER_TYPE_ATTRIBUTE, properties, emitter_type, FallbackMode::SET_DEFAULT);

    FindAttribute(SIZE_ATTRIBUTE, properties, generator_properties.emit_area, FallbackMode::SET_DEFAULT);
    FindAttribute(GRADIENT4_ATTRIBUTE, properties, generator_properties.color_gradient, FallbackMode::SET_DEFAULT);
    FindAttribute(DIRECTION_INTERVAL_ATTRIBUTE, properties, generator_properties.direction_degrees_interval, FallbackMode::SET_DEFAULT);
    FindAttribute(MAGNITUDE_INTERVAL_ATTRIBUTE, properties, generator_properties.magnitude_interval, FallbackMode::SET_DEFAULT);
    FindAttribute(ANGLAR_VELOCITY_INTERVAL_ATTRIBUTE, properties, generator_properties.angular_velocity_interval, FallbackMode::SET_DEFAULT);
    FindAttribute(LIFE_INTERVAL_ATTRIBUTE, properties, generator_properties.life_interval, FallbackMode::SET_DEFAULT);
    FindAttribute(START_SIZE_SPREAD_ATTRIBUTE, properties, generator_properties.start_size_spread, FallbackMode::SET_DEFAULT);
    FindAttribute(END_SIZE_SPREAD_ATTRIBUTE, properties, generator_properties.end_size_spread, FallbackMode::SET_DEFAULT);

    mono::ParticleSystem* particle_system = context->GetSystem<mono::ParticleSystem>();
    const std::vector<mono::ParticleEmitterComponent*>& attached_emitters = particle_system->GetAttachedEmitters(entity->id);
    if(attached_emitters.empty())
    {
        particle_system->AttachAreaEmitter(
            entity->id, duration, emit_rate, mono::EmitterType(emitter_type), generator_properties);
    }
    else
    {
        mono::ParticleEmitterComponent* emitter = attached_emitters.front();
        emitter->duration = duration;
        emitter->emit_rate = emit_rate;
        emitter->type = mono::EmitterType(emitter_type);

        particle_system->SetGeneratorProperties(emitter, generator_properties);
        particle_system->RestartEmitter(emitter);
    }

    return true;
}

void shared::RegisterSharedComponents(mono::IEntityManager* entity_manager)
{
    entity_manager->RegisterComponent(TRANSFORM_COMPONENT, CreateTransform, ReleaseTransform, UpdateTransform, GetTransform);
    entity_manager->RegisterComponent(SPRITE_COMPONENT, CreateSprite, ReleaseSprite, UpdateSprite);
    entity_manager->RegisterComponent(TEXT_COMPONENT, CreateText, ReleaseText, UpdateText);
    entity_manager->RegisterComponent(PATH_COMPONENT, CreatePath, ReleasePath, UpdatePath);
    entity_manager->RegisterComponent(ROAD_COMPONENT, CreateRoad, ReleaseRoad, UpdateRoad);
    entity_manager->RegisterComponent(LIGHT_COMPONENT, CreateLight, ReleaseLight, UpdateLight);
    entity_manager->RegisterComponent(PARTICLE_SYSTEM_COMPONENT, CreateParticleSystem, ReleaseParticleSystem, UpdateParticleSystem);
    entity_manager->RegisterComponent(AREA_EMITTER_COMPONENT, CreateBoxEmitter, ReleaseBoxEmitter, UpdateBoxEmitter);
}
