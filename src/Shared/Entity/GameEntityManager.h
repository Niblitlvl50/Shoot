
#pragma once

#include "EntitySystem/EntityManager.h"

namespace shared
{
    class GameEntityManager : public mono::EntityManager
    {
    public:

        using EntityManager::EntityManager;
        mono::EntityData LoadEntityFile(const char* entity_file) const override;
        const char* ComponentNameFromHash(uint32_t component_hash) const override;
    };
}
