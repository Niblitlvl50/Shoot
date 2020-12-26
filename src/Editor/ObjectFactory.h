
#pragma once

#include "ObjectProxies/IObjectProxy.h"
#include <string>

namespace editor
{
    class Editor;

    class ObjectFactory
    {
    public:

        ObjectFactory(Editor* editor);
        IObjectProxyPtr CreatePath(const std::string& name, const std::vector<math::Vector>& points) const;

    private:

        Editor* m_editor;
    };
}
