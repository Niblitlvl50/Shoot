
#pragma once

#include "ObjectProxies/IObjectVisitor.h"
#include "WorldFile.h"

#include "nlohmann/json.hpp"

#include <vector>
#include <string>

namespace editor
{
    class JsonSerializer : public IObjectVisitor
    {
    public:

        void WriteComponentEntities(const std::string& file_path, const game::LevelMetadata& level_metadata) const;

    private:

        void Accept(PathProxy* proxy) override;
        void Accept(ComponentProxy* proxy) override;

        std::vector<std::string> m_path_names;
        nlohmann::json m_json_entities_components;
    };
}
