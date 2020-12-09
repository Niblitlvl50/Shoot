
#pragma once

#include "MonoFwd.h"
#include "Entity/IEntityLogic.h"
#include "Math/Vector.h"

namespace game
{
    class ThrowableLogic : public game::IEntityLogic
    {
    public:

        ThrowableLogic(
            uint32_t entity_id,
            const char* spawned_entity,
            const math::Vector& target,
            mono::TransformSystem* transform_system,
            mono::IEntityManager* entity_manager);
        void Update(const mono::UpdateContext& update_context) override;

    private:

        const uint32_t m_entity_id;
        const char* m_spawned_entity;
        const math::Vector m_target;
        mono::TransformSystem* m_transform_system;
        mono::IEntityManager* m_entity_manager;

        math::Vector m_start_position;
        math::Vector m_move_delta;
        uint32_t m_move_timer;
    };
}
