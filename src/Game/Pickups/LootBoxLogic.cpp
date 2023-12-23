
#include "LootBoxLogic.h"
#include "EntitySystem/IEntityManager.h"
#include "InteractionSystem/InteractionSystem.h"

namespace tweak_values
{
    //constexpr float alive_time_s = 10.0f;
}

using namespace game;

LootBoxLogic::LootBoxLogic(uint32_t entity_id, game::InteractionSystem* interaction_system, mono::IEntityManager* entity_manager)
    : m_entity_id(entity_id)
    , m_interaction_system(interaction_system)
    , m_entity_manager(entity_manager)
    , m_alive_timer_s(0.0f)
{
    const InteractionCallback callback = [](uint32_t entity_id, InteractionType type) {
        //entity_manager->ReleaseEntity(entity_id);
    };
    m_interaction_system->SetInteractionCallback(entity_id, callback);
}

void LootBoxLogic::Update(const mono::UpdateContext& update_context)
{
    m_alive_timer_s += update_context.delta_s;

//    if(m_alive_timer_s > tweak_values::alive_time_s)
//        m_entity_manager->ReleaseEntity(m_entity_id);
}
