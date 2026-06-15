
#include "StatusEffectSystem.h"

#include "Physics/IBody.h"
#include "Physics/PhysicsSystem.h"

#include <vector>

using namespace game;

StatusEffectSystem::StatusEffectSystem(mono::PhysicsSystem* physics_system)
    : m_physics_system(physics_system)
{ }

void StatusEffectSystem::ApplySlowEffect(uint32_t entity_id, float multiplier, float duration_s)
{
    SlowEffect& effect = m_slow_effects[entity_id];
    effect.multiplier = multiplier;
    effect.remaining_s = duration_s;
}

void StatusEffectSystem::ClearEffects(uint32_t entity_id)
{
    m_slow_effects.erase(entity_id);
}

const char* StatusEffectSystem::Name() const
{
    return "statuseffectsystem";
}

void StatusEffectSystem::Update(const mono::UpdateContext& update_context)
{
    std::vector<uint32_t> expired;

    for(auto& [entity_id, effect] : m_slow_effects)
    {
        mono::IBody* body = m_physics_system->GetBody(entity_id);
        if(body)
            body->SetVelocity(body->GetVelocity() * effect.multiplier);

        effect.remaining_s -= update_context.delta_s;
        if(effect.remaining_s <= 0.0f)
            expired.push_back(entity_id);
    }

    for(uint32_t id : expired)
        m_slow_effects.erase(id);
}
