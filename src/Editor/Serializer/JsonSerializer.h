
#pragma once

#include "ObjectProxies/IObjectVisitor.h"

#include "nlohmann/json.hpp"

#include <vector>
#include <string>

namespace editor
{
    class JsonSerializer : public IObjectVisitor
    {
    public:

        void WriteEntities(const std::string& file_path) const;
        void WriteComponentEntities(const std::string& file_path) const;
        void WritePathFile(const std::string& file_path) const;
        void WritePolygons(const std::string& file_path) const;

    private:

        void Accept(PathProxy* proxy) override;
        void Accept(PolygonProxy* proxy) override;
        void Accept(ComponentProxy* proxy) override;

        std::vector<std::string> m_path_names;
        nlohmann::json m_json_entities;
        nlohmann::json m_json_entities_components;
        nlohmann::json m_json_polygons;
    };
}
