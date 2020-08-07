
#include "ModificationSystem.h"
#include "TriggerSystem.h"

#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Sprite/Sprite.h"
#include "TransformSystem/TransformSystem.h"
#include "Util/Hash.h"
#include "Util/Algorithm.h"

using namespace game;

namespace
{
    constexpr uint32_t NO_CALLBACK_SET = std::numeric_limits<uint32_t>::max();
}

ModificationSystem::ModificationSystem(
    uint32_t n, TriggerSystem* trigger_system, mono::TransformSystem* transform_system, mono::SpriteSystem* sprite_system)
    : m_trigger_system(trigger_system)
    , m_transform_system(transform_system)
    , m_sprite_system(sprite_system)
    , m_modification_components(n)
{
    m_containers.resize(n);
    m_active_containers.resize(n, false);
}

void ModificationSystem::AllocateContainer(uint32_t id)
{
    m_active_containers[id] = true;
}

void ModificationSystem::ReleaseContainer(uint32_t id)
{
    ModificationContainer& container = m_containers[id];
    for(const ModificationType* type : container.modifications)
    {
        m_trigger_system->RemoveTriggerCallback(type->trigger_hash, type->callback_id);
        m_modification_components.ReleasePoolData(type);
    }

    container.modifications.clear();

    m_active_containers[id] = false;
}

bool ModificationSystem::IsContainerAllocated(uint32_t id)
{
    return m_active_containers[id];
}

void ModificationSystem::AddAnimationComponent(uint32_t container_id, uint32_t trigger_hash, uint32_t animation_index)
{
    if(!IsContainerAllocated(container_id))
        AllocateContainer(container_id);

    ModificationType* type = m_modification_components.GetPoolData();
    type->target_id = container_id;
    type->trigger_hash = trigger_hash;
    type->duration = 0.0f;
    type->duration_counter = 0.0f;
    type->ease_function = math::EaseInOutCubic;
    type->is_initialized = true;
    type->type = ModificationType::Type::ANIMATION;
    type->animation_index = animation_index;

    const TriggerCallback callback = [this, type](uint32_t trigger_id, TriggerState state) {
        m_active_modifications.push_back(type);
    };

    type->callback_id = m_trigger_system->RegisterTriggerCallback(trigger_hash, callback);

    ModificationContainer& container = m_containers[container_id];
    container.modifications.push_back(type);
}

void ModificationSystem::AddTranslationComponent(
    uint32_t container_id, uint32_t trigger_hash, float duration, math::EaseFunction func, const math::Vector& translation_delta)
{
    if(!IsContainerAllocated(container_id))
        AllocateContainer(container_id);

    ModificationType* type = m_modification_components.GetPoolData();
    type->target_id = container_id;
    type->trigger_hash = trigger_hash;
    type->duration = duration;
    type->duration_counter = 0.0f;
    type->ease_function = func;
    type->is_initialized = false;
    type->type = ModificationType::Type::TRANSLATION;
    
    type->translation.delta_x = translation_delta.x;
    type->translation.delta_y = translation_delta.y;

    const TriggerCallback callback = [this, type](uint32_t trigger_hash, TriggerState state) {
        m_active_modifications.push_back(type);
    };

    type->callback_id = m_trigger_system->RegisterTriggerCallback(trigger_hash, callback);

    ModificationContainer& container = m_containers[container_id];
    container.modifications.push_back(type);
}

void ModificationSystem::AddRotationComponent(
    uint32_t container_id, uint32_t trigger_hash, float duration, math::EaseFunction func, float rotation_delta)
{
    if(!IsContainerAllocated(container_id))
        AllocateContainer(container_id);

    ModificationType* type = m_modification_components.GetPoolData();
    type->target_id = container_id;
    type->trigger_hash = trigger_hash;
    type->duration = duration;
    type->duration_counter = 0.0f;
    type->ease_function = func;
    type->is_initialized = false;
    type->type = ModificationType::Type::ROTATION;

    type->rotation.delta_rotation = rotation_delta;

    const TriggerCallback callback = [this, type](uint32_t trigger_id, TriggerState state) {
        m_active_modifications.push_back(type);
    };
    type->callback_id = m_trigger_system->RegisterTriggerCallback(trigger_hash, callback);

    ModificationContainer& container = m_containers[container_id];
    container.modifications.push_back(type);
}

uint32_t ModificationSystem::Id() const
{
    return mono::Hash(Name());
}

const char* ModificationSystem::Name() const
{
    return "ModificationSystem";
}

uint32_t ModificationSystem::Capacity() const
{
    return m_containers.capacity();
}

void ModificationSystem::Update(const mono::UpdateContext& update_context)
{
    for(ModificationType* type : m_active_modifications)
    {
        if(type->callback_id != NO_CALLBACK_SET)
        {
            m_trigger_system->RemoveTriggerCallback(type->trigger_hash, type->callback_id);
            type->callback_id = NO_CALLBACK_SET;
        }

        switch(type->type)
        {
        case ModificationType::Type::ANIMATION:
        {
            mono::Sprite* sprite = m_sprite_system->GetSprite(type->target_id);
            sprite->SetAnimation(type->animation_index);
            break;
        }
        case ModificationType::Type::TRANSLATION:
        {
            math::Matrix& transform = m_transform_system->GetTransform(type->target_id);
    
            if(!type->is_initialized)
            {
                const math::Vector& position = math::GetPosition(transform);
                type->translation.start_x = position.x;
                type->translation.start_y = position.y;

                type->is_initialized = true;
            }

            const math::Vector new_position(
                type->ease_function(type->duration_counter, type->duration, type->translation.start_x, type->translation.delta_x),
                type->ease_function(type->duration_counter, type->duration, type->translation.start_y, type->translation.delta_y)
            );

            math::Position(transform, new_position);
            m_transform_system->SetTransformState(type->target_id, mono::TransformState::CLIENT);
            break;
        }
        case ModificationType::Type::ROTATION:
        {
            math::Matrix& transform = m_transform_system->GetTransform(type->target_id);
            const math::Vector position = math::GetPosition(transform);

            if(!type->is_initialized)
            {
                type->rotation.start_rotation = math::GetZRotation(transform);
                type->is_initialized = true;
            }

            const float new_rotation =
                type->ease_function(type->duration_counter, type->duration, type->rotation.start_rotation, type->rotation.delta_rotation);

            transform = math::CreateMatrixFromZRotation(new_rotation);
            math::Position(transform, position);
            m_transform_system->SetTransformState(type->target_id, mono::TransformState::CLIENT);
            break;
        }
        }

        type->duration_counter += (float(update_context.delta_ms) / 1000.0f);
    }

    const auto remove_if_done = [](ModificationType* type) {
        return (type->duration - type->duration_counter) <= 0.0f;
    };
    mono::remove_if(m_active_modifications, remove_if_done);
}
