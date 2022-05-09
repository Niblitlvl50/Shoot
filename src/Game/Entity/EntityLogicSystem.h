
#pragma once

#include "IGameSystem.h"
#include "Util/ActiveVector.h"

#include <cstddef>
#include <vector>

namespace game
{
    class IEntityLogic;

    struct EntityLogicComponent
    {
        IEntityLogic* logic;
    };

    class EntityLogicSystem : public mono::IGameSystem
    {
    public:

        EntityLogicSystem(size_t n_entities);
        ~EntityLogicSystem();

        void AddLogic(uint32_t entity_id, IEntityLogic* entity_logic);
        void ReleaseLogic(uint32_t entity_id);

        uint32_t Id() const override;
        const char* Name() const override;
        void Update(const mono::UpdateContext& update_context) override;

        mono::ActiveVector<EntityLogicComponent> m_logics;
    };
}
