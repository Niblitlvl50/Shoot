
#include "EntityRepository.h"
#include "Prefabs.h"

using namespace editor;

bool EntityRepository::LoadDefinitions()
{
    m_prefabs = LoadPrefabDefinitions();
    return true;
}

const PrefabDefinition* EntityRepository::GetPrefabFromName(const std::string& name) const
{
    const auto find_func = [&name](const PrefabDefinition& definition) {
        return definition.name == name;
    };

    const auto it = std::find_if(m_prefabs.begin(), m_prefabs.end(), find_func);
    return it != m_prefabs.end() ? &*it : nullptr;
}
