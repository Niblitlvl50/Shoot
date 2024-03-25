
#include "AnimationSystem.h"
#include "TriggerSystem/TriggerSystem.h"

#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Sprite/Sprite.h"
#include "TransformSystem/TransformSystem.h"
#include "System/Hash.h"
#include "Util/Algorithm.h"

using namespace game;

namespace
{
    constexpr uint32_t NO_CALLBACK_SET = std::numeric_limits<uint32_t>::max();
}

AnimationSystem::AnimationSystem(
    uint32_t n, TriggerSystem* trigger_system, mono::TransformSystem* transform_system, mono::SpriteSystem* sprite_system)
    : m_trigger_system(trigger_system)
    , m_transform_system(transform_system)
    , m_sprite_system(sprite_system)
    , m_sprite_anim_pool(64)
    , m_transform_anim_pool(64)
{
    m_animation_containers.resize(n);
    m_active_animation_containers.resize(n, false);
}

AnimationContainer* AnimationSystem::AllocateAnimationContainer(uint32_t entity_id)
{
    m_active_animation_containers[entity_id] = true;
    
    AnimationContainer& allocated_container = m_animation_containers[entity_id];
    allocated_container.ref_counter++;

    return &allocated_container;
}

void AnimationSystem::ReleaseAnimationContainer(uint32_t entity_id)
{
    AnimationContainer& allocated_container = m_animation_containers[entity_id];
    allocated_container.ref_counter--;
    if(allocated_container.ref_counter > 0)
        return;

    for(SpriteAnimationComponent* component : allocated_container.sprite_components)
    {
        if(component->callback_id != NO_CALLBACK_SET)
            m_trigger_system->RemoveTriggerCallback(component->trigger_hash, component->callback_id, entity_id);

        mono::remove(m_sprite_anims_to_process, component);
        m_sprite_anim_pool.ReleasePoolData(component);
    }

    for(TransformAnimationComponent* component : allocated_container.transform_components)
    {
        if(component->callback_id != NO_CALLBACK_SET)
            m_trigger_system->RemoveTriggerCallback(component->trigger_hash, component->callback_id, entity_id);

        mono::remove(m_transform_anims_to_process, component);
        m_transform_anim_pool.ReleasePoolData(component);
    }

    allocated_container.sprite_components.clear();
    allocated_container.transform_components.clear();

    m_active_animation_containers[entity_id] = false;
}

bool AnimationSystem::IsAnimationContainerAllocated(uint32_t entity_id)
{
    return m_active_animation_containers[entity_id];
}

 SpriteAnimationComponent* AnimationSystem::AddSpriteAnimation(
     uint32_t container_id, uint32_t trigger_hash, uint32_t animation_index)
{
    SpriteAnimationComponent* allocated_component = m_sprite_anim_pool.GetPoolData();
    allocated_component->target_id = container_id;
    allocated_component->trigger_hash = trigger_hash;
    allocated_component->animation_index = animation_index;

    const TriggerCallback callback = [this, allocated_component](uint32_t trigger_id) {
        m_sprite_anims_to_process.push_back(allocated_component);
    };
    allocated_component->callback_id = m_trigger_system->RegisterTriggerCallback(trigger_hash, callback, container_id);

    AnimationContainer& allocated_container = m_animation_containers[container_id];
    allocated_container.sprite_components.push_back(allocated_component);

    m_animation_containers[container_id].sprite_components.push_back(allocated_component);
    return allocated_component;
}

TransformAnimationComponent* AnimationSystem::AddTransformComponent(
    uint32_t container_id,
    uint32_t trigger_hash,
    float duration,
    math::EaseFunction func_x,
    math::EaseFunction func_y,
    AnimationMode mode,
    TransformAnimType type,
    const math::Vector& translation_delta)
{
    TransformAnimationComponent* allocated_component = m_transform_anim_pool.GetPoolData();
    allocated_component->target_id = container_id;
    allocated_component->trigger_hash = trigger_hash;
    allocated_component->callback_id = NO_CALLBACK_SET;
    allocated_component->duration = duration;
    allocated_component->duration_counter = 0.0f;
    allocated_component->ease_function_x = func_x;
    allocated_component->ease_function_y = func_y;
    allocated_component->animation_flags = mode;
    allocated_component->transform_type = type;
    allocated_component->is_initialized = false;

    allocated_component->delta_x = translation_delta.x;
    allocated_component->delta_y = translation_delta.y;

    if(mode & AnimationMode::TRIGGER_ACTIVATED)
    {
        const TriggerCallback callback = [this, allocated_component](uint32_t trigger_id) {
            AddTransformAnimatonToUpdate(allocated_component);
        };
        allocated_component->callback_id = m_trigger_system->RegisterTriggerCallback(trigger_hash, callback, container_id);
    }
    else
    {
        AddTransformAnimatonToUpdate(allocated_component);
    }

    m_animation_containers[container_id].transform_components.push_back(allocated_component);
    return allocated_component;
}

TransformAnimationComponent* AnimationSystem::AddTranslationComponent(
    uint32_t container_id, uint32_t trigger_hash, float duration, math::EaseFunction func_x, math::EaseFunction func_y, AnimationMode mode, const math::Vector& translation_delta)
{
    return AddTransformComponent(container_id, trigger_hash, duration, func_x, func_y, mode, TransformAnimType::TRANSLATION, translation_delta);
}

TransformAnimationComponent* AnimationSystem::AddRotationComponent(
    uint32_t container_id, uint32_t trigger_hash, float duration, math::EaseFunction func, AnimationMode mode, float rotation_delta)
{
    return AddTransformComponent(container_id, trigger_hash, duration, func, func, mode, TransformAnimType::ROTATION, math::Vector(rotation_delta, 0.0f));
}

TransformAnimationComponent* AnimationSystem::AddScaleComponent(
    uint32_t container_id, uint32_t trigger_hash, float duration, math::EaseFunction func, AnimationMode mode, float scale_delta)
{
    return AddTransformComponent(container_id, trigger_hash, duration, func, func, mode, TransformAnimType::SCALE, math::Vector(scale_delta, 0.0f));
}

void AnimationSystem::AddTransformAnimatonToUpdate(TransformAnimationComponent* transform_animation)
{
    const bool is_in_update = mono::contains(m_transform_anims_to_process, transform_animation);
    if(!is_in_update)
        m_transform_anims_to_process.push_back(transform_animation);
}

const char* AnimationSystem::Name() const
{
    return "AnimationSystem";
}

void AnimationSystem::Update(const mono::UpdateContext& update_context)
{
    for(SpriteAnimationComponent* sprite_anim : m_sprite_anims_to_process)
    {
        mono::Sprite* sprite = m_sprite_system->GetSprite(sprite_anim->target_id);
        sprite->SetAnimation(sprite_anim->animation_index);
    }

    m_sprite_anims_to_process.clear();


    const auto process_transform_anims_func = [this, &update_context](TransformAnimationComponent* transform_anim) {

        // Do the stuff
        math::Matrix& transform = m_transform_system->GetTransform(transform_anim->target_id);
        const math::Vector& position = math::GetPosition(transform);

        if(!transform_anim->is_initialized)
        {
            if(transform_anim->transform_type == TransformAnimType::TRANSLATION)
            {
                transform_anim->start_x = position.x;
                transform_anim->start_y = position.y;
            }
            else if(transform_anim->transform_type == TransformAnimType::ROTATION)
            {
                transform_anim->start_x = math::GetZRotation(transform);
            }
            else
            {
                transform_anim->start_x = 1.0f;
            }

            transform_anim->is_initialized = true;
        }

        transform_anim->duration_counter += update_context.delta_s;
        transform_anim->duration_counter = std::clamp(transform_anim->duration_counter, 0.0f, transform_anim->duration);

        if(transform_anim->transform_type == TransformAnimType::TRANSLATION)
        {
            const math::Vector new_position(
                transform_anim->ease_function_x(transform_anim->duration_counter, transform_anim->duration, transform_anim->start_x, transform_anim->delta_x),
                transform_anim->ease_function_y(transform_anim->duration_counter, transform_anim->duration, transform_anim->start_y, transform_anim->delta_y)
            );
            math::Position(transform, new_position);
            transform_anim->current_x = new_position.x;
            transform_anim->current_y = new_position.y;
        }
        else if(transform_anim->transform_type == TransformAnimType::ROTATION)
        {
            const float new_rotation =
                transform_anim->ease_function_x(transform_anim->duration_counter, transform_anim->duration, transform_anim->start_x, transform_anim->delta_x);
            transform = math::CreateMatrixWithPositionRotation(position, new_rotation);
            transform_anim->current_x = new_rotation;
        }
        else
        {
            const float new_scale =
                transform_anim->ease_function_x(transform_anim->duration_counter, transform_anim->duration, transform_anim->start_x, transform_anim->delta_x);
            transform = math::CreateMatrixWithPositionScale(position, new_scale);
            transform_anim->current_x = new_scale;
        }

        m_transform_system->SetTransformState(transform_anim->target_id, mono::TransformState::CLIENT);

        // Check if done
        bool is_done = (transform_anim->duration - transform_anim->duration_counter) <= 0.0f;
        
        const bool ping_pong = (transform_anim->animation_flags & AnimationMode::PING_PONG);
        const bool reversable = (transform_anim->animation_flags & AnimationMode::TRIGGER_REVERSE);
        const bool looping = (transform_anim->animation_flags & AnimationMode::LOOPING);

        if(is_done && (ping_pong || reversable))
        {
            transform_anim->duration_counter = 0.0f;

            transform_anim->delta_x = -transform_anim->delta_x;
            transform_anim->delta_y = -transform_anim->delta_y;
            transform_anim->start_x = transform_anim->current_x;
            transform_anim->start_y = transform_anim->current_y;

            is_done = ping_pong ? false : true;
        }
        else if(is_done && looping)
        {
            transform_anim->duration_counter = 0.0f;
            is_done = false;
        }

        if(transform_anim->animation_flags & AnimationMode::ONE_SHOT && transform_anim->callback_id != NO_CALLBACK_SET)
        {
            m_trigger_system->RemoveTriggerCallback(
                transform_anim->trigger_hash, transform_anim->callback_id, transform_anim->target_id);
            transform_anim->callback_id = NO_CALLBACK_SET;
        }

        if(is_done)
        {
            transform_anim->is_initialized = false;
            transform_anim->duration_counter = 0.0f;
        }

        return is_done;
    };
    
    mono::remove_if(m_transform_anims_to_process, process_transform_anims_func);
}
