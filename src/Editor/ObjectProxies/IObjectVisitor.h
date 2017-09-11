
#pragma once

namespace editor
{
    class IObjectVisitor
    {
    public:

        virtual ~IObjectVisitor()
        { }

        virtual void Accept(class EntityProxy* proxy) = 0;
        virtual void Accept(class PathProxy* proxy) = 0;
        virtual void Accept(class PolygonProxy* proxy) = 0;
        virtual void Accept(class PrefabProxy* proxy) = 0;
    };
}
