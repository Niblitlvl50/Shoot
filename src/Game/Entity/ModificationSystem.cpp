
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

AnimationSystem::AnimationSystem(
    uint32_t n, TriggerSystem* trigger_system, mono::TransformSystem* transform_system, mono::SpriteSystem* sprite_system)
    : m_trigger_system(trigger_system)
    , m_transform_system(transform_system)
    , m_sprite_system(sprite_system)
{
    m_sprite_components.resize(n);
    m_active_sprite_components.resize(n, false);

    m_translation_components.resize(n);
    m_active_translation_components.resize(n, false);

    m_rotation_components.resize(n);
    m_active_rotation_components.resize(n, false);
}

SpriteAnimationComponent* AnimationSystem::AllocateSpriteAnimation(uint32_t entity_id)
{
    m_active_sprite_components[entity_id] = true;
    
    SpriteAnimationComponent* allocated_component = &m_sprite_components[entity_id];
    allocated_component->callback_id = NO_CALLBACK_SET;
    return allocated_component;
}

void AnimationSystem::ReleaseSpriteAnimation(uint32_t entity_id)
{
    SpriteAnimationComponent& allocated_component = m_sprite_components[entity_id];
    if(allocated_component.callback_id != NO_CALLBACK_SET)
        m_trigger_system->RemoveTriggerCallback(allocated_component.trigger_hash, allocated_component.callback_id);

    m_active_sprite_components[entity_id] = false;
}

void AnimationSystem::AddAnimationComponent(uint32_t entity_id, uint32_t trigger_hash, uint32_t animation_index)
{
    SpriteAnimationComponent* sprite_animation = &m_sprite_components[entity_id];

    if(sprite_animation->callback_id != NO_CALLBACK_SET)
        m_trigger_system->RemoveTriggerCallback(sprite_animation->trigger_hash, sprite_animation->callback_id);

    sprite_animation->target_id = entity_id;
    sprite_animation->trigger_hash = trigger_hash;
    sprite_animation->animation_index = animation_index;

    const TriggerCallback callback = [this, sprite_animation](uint32_t trigger_id, TriggerState state) {
        m_sprite_anims_to_process.push_back(sprite_animation);
    };

    sprite_animation->callback_id = m_trigger_system->RegisterTriggerCallback(trigger_hash, callback);
}

TranslateAnimationComponent* AnimationSystem::AllocateTranslationAnimation(uint32_t entity_id)
{
    m_active_translation_components[entity_id] = true;
    
    TranslateAnimationComponent* allocated_component = &m_translation_components[entity_id];
    allocated_component->callback_id = NO_CALLBACK_SET;
    return allocated_component;
}

void AnimationSystem::ReleaseTranslationAnimation(uint32_t entity_id)
{
    TranslateAnimationComponent& allocated_component = m_translation_components[entity_id];
    if(allocated_component.callback_id != NO_CALLBACK_SET)
        m_trigger_system->RemoveTriggerCallback(allocated_component.trigger_hash, allocated_component.callback_id);

    mono::remove(m_translation_anims_to_process, &allocated_component);

    m_active_translation_components[entity_id] = false;
}

void AnimationSystem::AddTranslationComponent(
    uint32_t entity_id, uint32_t trigger_hash, float duration, math::EaseFunction func, shared::AnimationMode mode, const math::Vector& translation_delta)
{
    TranslateAnimationComponent* translation_anim = &m_translation_components[entity_id];

    if(translation_anim->callback_id != NO_CALLBACK_SET)
        m_trigger_system->RemoveTriggerCallback(translation_anim->trigger_hash, translation_anim->callback_id);

    mono::remove(m_translation_anims_to_process, translation_anim);

    translation_anim->target_id = entity_id;
    translation_anim->trigger_hash = trigger_hash;
    translation_anim->callback_id = NO_CALLBACK_SET;
    translation_anim->duration = duration;
    translation_anim->duration_counter = 0.0f;
    translation_anim->ease_function = func;
    translation_anim->is_initialized = false;
    translation_anim->animation_flags = mode;

    translation_anim->delta_x = translation_delta.x;
    translation_anim->delta_y = translation_delta.y;

    if(mode & shared::AnimationMode::TRIGGER_ACTIVATED)
    {
        const TriggerCallback callback = [this, translation_anim](uint32_t trigger_id, TriggerState state) {
            m_translation_anims_to_process.push_back(translation_anim);
        };
        translation_anim->callback_id = m_trigger_system->RegisterTriggerCallback(trigger_hash, callback);
    }
    else
    {
        m_translation_anims_to_process.push_back(translation_anim);
    }
}

RotationAnimationComponent* AnimationSystem::AllocateRotationAnimation(uint32_t entity_id)
{
    m_active_rotation_components[entity_id] = true;
    
    RotationAnimationComponent* allocated_component = &m_rotation_components[entity_id];
    allocated_component->callback_id = NO_CALLBACK_SET;
    return allocated_component;
}

void AnimationSystem::ReleaseRotationAnimation(uint32_t entity_id)
{
    RotationAnimationComponent& allocated_component = m_rotation_components[entity_id];
    if(allocated_component.callback_id != NO_CALLBACK_SET)
        m_trigger_system->RemoveTriggerCallback(allocated_component.trigger_hash, allocated_component.callback_id);

    mono::remove(m_rotation_anims_to_process, &allocated_component);

    m_active_rotation_components[entity_id] = false;
}

void AnimationSystem::AddRotationComponent(
    uint32_t entity_id, uint32_t trigger_hash, float duration, math::EaseFunction func, shared::AnimationMode mode, float rotation_delta)
{
    RotationAnimationComponent* rotation_anim = &m_rotation_components[entity_id];

    if(rotation_anim->callback_id != NO_CALLBACK_SET)
        m_trigger_system->RemoveTriggerCallback(rotation_anim->trigger_hash, rotation_anim->callback_id);

    mono::remove(m_rotation_anims_to_process, rotation_anim);
    
    rotation_anim->target_id = entity_id;
    rotation_anim->trigger_hash = trigger_hash;
    rotation_anim->callback_id = NO_CALLBACK_SET;
    rotation_anim->duration = duration;
    rotation_anim->duration_counter = 0.0f;
    rotation_anim->ease_function = func;
    rotation_anim->is_initialized = false;
    rotation_anim->animation_flags = mode;

    rotation_anim->delta_rotation = rotation_delta;

    if(mode & shared::AnimationMode::TRIGGER_ACTIVATED)
    {
        const TriggerCallback callback = [this, rotation_anim](uint32_t trigger_id, TriggerState state) {
            m_rotation_anims_to_process.push_back(rotation_anim);
        };
        rotation_anim->callback_id = m_trigger_system->RegisterTriggerCallback(trigger_hash, callback);
    }
    else
    {
        m_rotation_anims_to_process.push_back(rotation_anim);
    }
}

uint32_t AnimationSystem::Id() const
{
    return mono::Hash(Name());
}

const char* AnimationSystem::Name() const
{
    return "AnimationSystem";
}

uint32_t AnimationSystem::Capacity() const
{
    return m_sprite_components.capacity();
}

void AnimationSystem::Update(const mono::UpdateContext& update_context)
{
    for(SpriteAnimationComponent* sprite_anim : m_sprite_anims_to_process)
    {
        m_trigger_system->RemoveTriggerCallback(sprite_anim->trigger_hash, sprite_anim->callback_id);
        sprite_anim->callback_id = NO_CALLBACK_SET;

        mono::Sprite* sprite = m_sprite_system->GetSprite(sprite_anim->target_id);
        sprite->SetAnimation(sprite_anim->animation_index);
    }

    m_sprite_anims_to_process.clear();


    const auto process_translation_anims_func = [this, &update_context](TranslateAnimationComponent* translation_anim) {

        if(translation_anim->callback_id != NO_CALLBACK_SET)
        {
            m_trigger_system->RemoveTriggerCallback(translation_anim->trigger_hash, translation_anim->callback_id);
            translation_anim->callback_id = NO_CALLBACK_SET;
        }

        // Do the stuff
        math::Matrix& transform = m_transform_system->GetTransform(translation_anim->target_id);

        if(!translation_anim->is_initialized)
        {
            const math::Vector& position = math::GetPosition(transform);
            translation_anim->start_x = position.x;
            translation_anim->start_y = position.y;

            translation_anim->is_initialized = true;
        }

        const math::Vector new_position(
            translation_anim->ease_function(translation_anim->duration_counter, translation_anim->duration, translation_anim->start_x, translation_anim->delta_x),
            translation_anim->ease_function(translation_anim->duration_counter, translation_anim->duration, translation_anim->start_y, translation_anim->delta_y)
        );

        math::Position(transform, new_position);
        m_transform_system->SetTransformState(translation_anim->target_id, mono::TransformState::CLIENT);
    
        translation_anim->duration_counter += (float(update_context.delta_ms) / 1000.0f);

        // Check if done
        const bool is_done = (translation_anim->duration - translation_anim->duration_counter) <= 0.0f;
        if(is_done && (translation_anim->animation_flags & shared::AnimationMode::PING_PONG))
        {
            translation_anim->is_initialized = false;
            translation_anim->duration_counter = 0.0f;
            
            translation_anim->delta_x = -translation_anim->delta_x;
            translation_anim->delta_y = -translation_anim->delta_y;

            return false;
        }

        return is_done;
    };
    mono::remove_if(m_translation_anims_to_process, process_translation_anims_func);


    const auto process_rotation_anims_func = [this, &update_context](RotationAnimationComponent* rotation_anim) {

        if(rotation_anim->callback_id != NO_CALLBACK_SET)
        {
            m_trigger_system->RemoveTriggerCallback(rotation_anim->trigger_hash, rotation_anim->callback_id);
            rotation_anim->callback_id = NO_CALLBACK_SET;
        }

        // Do the stuff
        math::Matrix& transform = m_transform_system->GetTransform(rotation_anim->target_id);
        const math::Vector position = math::GetPosition(transform);

        if(!rotation_anim->is_initialized)
        {
            rotation_anim->start_rotation = math::GetZRotation(transform);
            rotation_anim->is_initialized = true;
        }

        const float new_rotation =
            rotation_anim->ease_function(rotation_anim->duration_counter, rotation_anim->duration, rotation_anim->start_rotation, rotation_anim->delta_rotation);

        transform = math::CreateMatrixFromZRotation(new_rotation);
        math::Position(transform, position);
        m_transform_system->SetTransformState(rotation_anim->target_id, mono::TransformState::CLIENT);

        rotation_anim->duration_counter += (float(update_context.delta_ms) / 1000.0f);

        // Check if done
        const bool is_done = (rotation_anim->duration - rotation_anim->duration_counter) <= 0.0f;
        if(is_done && (rotation_anim->animation_flags & shared::AnimationMode::PING_PONG))
        {
            rotation_anim->is_initialized = false;
            rotation_anim->duration_counter = 0.0f;
            rotation_anim->delta_rotation = -rotation_anim->delta_rotation;

            return false;
        }

        return is_done;
    };
    mono::remove_if(m_rotation_anims_to_process, process_rotation_anims_func);
}
