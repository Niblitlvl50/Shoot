
#include "EnemyPickupLogic.h"
#include "EntitySystem/IEntityManager.h"

using namespace game;

namespace tweak_values
{
    constexpr float decoy_time_s = 10.0f;
}

EnemyPickupLogic::EnemyPickupLogic(uint32_t entity_id, mono::IEntityManager* entity_manager)
    : m_entity_id(entity_id)
    , m_entity_manager(entity_manager)
    , m_alive_timer_s(0.0f)
{ }

void EnemyPickupLogic::Update(const mono::UpdateContext& update_context)
{
    m_alive_timer_s += update_context.delta_s;

    if(m_alive_timer_s > tweak_values::decoy_time_s)
        m_entity_manager->ReleaseEntity(m_entity_id);
}
