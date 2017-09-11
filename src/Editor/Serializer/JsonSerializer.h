
#pragma once

#include "ObjectProxies/IObjectVisitor.h"

#include <vector>
#include <string>

namespace editor
{
    class JsonSerializer : public IObjectVisitor
    {
    public:

        JsonSerializer(const std::string& file_path);

        void WritePathFile();

    private:

        void Accept(EntityProxy* proxy) override;
        void Accept(PathProxy* proxy) override;
        void Accept(PolygonProxy* proxy) override;
        void Accept(PrefabProxy* proxy) override;

        const std::string m_file_path;
        std::vector<std::string> m_path_names;
    };

    class JsonDeserializer : public IObjectVisitor
    {
    public:

        JsonDeserializer(const std::string& file_path);

        void Accept(EntityProxy* proxy) override;
        void Accept(PathProxy* proxy) override;
        void Accept(PolygonProxy* proxy) override;
        void Accept(PrefabProxy* proxy) override;        
    };
}
