
#pragma once

#include "Prefabs.h"

#include <string>
#include <vector>

namespace editor
{
    class EntityRepository
    {
    public:

        bool LoadDefinitions();

        const PrefabDefinition* GetPrefabFromName(const std::string& name) const;
        std::vector<PrefabDefinition> m_prefabs;
    };
}
