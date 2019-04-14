
#pragma once

#include "IGameSystem.h"
#include <vector>

namespace game
{
    class IEntityLogic;

    class EntityLogicSystem : public mono::IGameSystem
    {
    public:

        EntityLogicSystem(size_t n_entities);
        ~EntityLogicSystem();

        void AddLogic(uint32_t entity_id, IEntityLogic* entity_logic);
        void ReleaseLogic(uint32_t entity_id);

        uint32_t Id() const override;
        const char* Name() const override;
        uint32_t Capacity() const override;
        void Update(uint32_t delta_ms) override;

        std::vector<IEntityLogic*> m_logics;
    };
}
