
#pragma once

namespace editor
{
    class IObjectVisitor
    {
    public:

        virtual ~IObjectVisitor()
        { }

        virtual void Accept(class PathProxy* proxy) = 0;
        virtual void Accept(class ComponentProxy* proxy) = 0;
    };
}
