
#pragma once

#include "ObjectProxies/IObjectVisitor.h"
#include "Prefab.h"

#include "nlohmann_json/json.hpp"

#include <vector>
#include <string>

namespace editor
{
    class JsonSerializer : public IObjectVisitor
    {
    public:

        void WriteEntities(const std::string& file_path) const;
        void WritePathFile(const std::string& file_path) const;
        void WritePrefabs(const std::string& file_path) const;

    private:

        void Accept(EntityProxy* proxy) override;
        void Accept(PathProxy* proxy) override;
        void Accept(PolygonProxy* proxy) override;
        void Accept(PrefabProxy* proxy) override;

        std::vector<std::string> m_path_names;
        std::vector<editor::PrefabData> m_prefabs;

        nlohmann::json m_json_entities;
    };
}
