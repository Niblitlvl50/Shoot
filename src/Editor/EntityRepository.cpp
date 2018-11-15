
#include "EntityRepository.h"
#include "System/File.h"
#include "Math/Serialize.h"
#include "Hash.h"
#include "Prefabs.h"

#include "nlohmann/json.hpp"

using namespace editor;

namespace
{
    bool LoadEntities(std::vector<EntityDefinition>& collection)
    {
        File::FilePtr file = File::OpenAsciiFile("res/editor_entities.json");
        if(!file)
            return false;

        std::vector<byte> file_data;
        File::FileRead(file, file_data);
        const nlohmann::json& json = nlohmann::json::parse(file_data);

        const nlohmann::json& entities = json["entities"];
        collection.reserve(entities.size());

        for(auto& object : entities)
        {
            const nlohmann::json& attribute_list = object["attribute_types"];

            EntityDefinition definition;
            definition.name = object["name"];
            definition.sprite_file = object["sprite"];
            definition.scale = object["scale"];
            definition.attribute_types.reserve(attribute_list.size());

            for(auto& attrib : attribute_list)
            {
                const std::string name = attrib;
                const unsigned int hash = mono::Hash(name.c_str());
                definition.attribute_types.push_back(hash);
            }

            collection.push_back(definition);
        }

        return true;
    }
}

bool EntityRepository::LoadDefinitions()
{
    m_prefabs = LoadPrefabDefinitions();
    return LoadEntities(m_entities);
}

const EntityDefinition* EntityRepository::GetDefinitionFromName(const std::string& name) const
{
    const auto find_func = [&name](const EntityDefinition& definition) {
        return definition.name == name;
    };

    const auto it = std::find_if(m_entities.begin(), m_entities.end(), find_func);    
    return it != m_entities.end() ? &*it : nullptr;
}

const PrefabDefinition* EntityRepository::GetPrefabFromName(const std::string& name) const
{
    const auto find_func = [&name](const PrefabDefinition& definition) {
        return definition.name == name;
    };

    const auto it = std::find_if(m_prefabs.begin(), m_prefabs.end(), find_func);
    return it != m_prefabs.end() ? &*it : nullptr;
}
