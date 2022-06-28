
#pragma once

namespace editor
{
    class IObjectVisitor
    {
    public:

        virtual ~IObjectVisitor()
        { }

        virtual void Accept(class ComponentProxy* proxy) = 0;
    };
}
