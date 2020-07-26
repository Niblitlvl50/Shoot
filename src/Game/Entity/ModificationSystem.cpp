
#include "ModificationSystem.h"
#include "TriggerSystem.h"

#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Sprite/Sprite.h"
#include "Util/Hash.h"

using namespace game;

namespace
{
    constexpr uint32_t NO_CALLBACK_SET = std::numeric_limits<uint32_t>::max();
}

ModificationSystem::ModificationSystem(uint32_t n, TriggerSystem* trigger_system, mono::SpriteSystem* sprite_system)
    : m_trigger_system(trigger_system)
    , m_sprite_system(sprite_system)
{
    m_animations.resize(n);
    m_active_animations.resize(n, false);
    m_callback_ids.resize(n, NO_CALLBACK_SET);
}

AnimationComponent* ModificationSystem::AllocateAnimationComponent(uint32_t id)
{
    m_active_animations[id] = true;
    return &m_animations[id];
}

void ModificationSystem::ReleaseAnimationComponent(uint32_t id)
{
    const uint32_t trigger_hash = m_animations[id].trigger_hash;
    const uint32_t callback_id = m_callback_ids[id];

    m_trigger_system->RemoveTriggerCallback(trigger_hash, callback_id);

    m_active_animations[id] = false;
    m_callback_ids[id] = NO_CALLBACK_SET;
}

void ModificationSystem::UpdateAnimationComponent(uint32_t id, const AnimationComponent& animation_data)
{
    m_animations[id] = animation_data;

    const AnimationComponent* animation_ptr = &m_animations[id];

    uint32_t& callback_id = m_callback_ids[id];
    if(callback_id == NO_CALLBACK_SET)
    {
        const TriggerCallback callback = [this, id, animation_ptr](uint32_t trigger_id, TriggerState state) {
            m_animations_to_update.push_back({ id, animation_ptr->animation_index });
        };

        callback_id = m_trigger_system->RegisterTriggerCallback(animation_data.trigger_hash, callback);
    }
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
    return m_animations.capacity();
}

void ModificationSystem::Update(const mono::UpdateContext& update_context)
{
    for(const auto& animation_data : m_animations_to_update)
    {
        mono::Sprite* sprite = m_sprite_system->GetSprite(animation_data.target_id);
        sprite->SetAnimation(animation_data.animation_index);
    }

    m_animations_to_update.clear();
}
