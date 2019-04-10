
#pragma once

#include "ObjectProxies/IObjectVisitor.h"
#include "WorldFile.h"

#include <string>

namespace editor
{
    class BinarySerializer : public IObjectVisitor
    {
    public:

        BinarySerializer();
        void WritePolygonFile(const std::string& file_name) const;
        void WriteObjects(const std::string& file_name) const;

    private:
        
        void Accept(PathProxy* proxy) override;
        void Accept(PolygonProxy* proxy) override;
        void Accept(PrefabProxy* proxy) override;
        void Accept(ComponentProxy* proxy) override;
        
        world::LevelFileHeader m_polygon_data;
        world::WorldObjectsHeader m_object_data;
    };
}
