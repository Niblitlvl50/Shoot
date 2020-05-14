
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
        IObjectProxyPtr CreatePolygon(
            const std::string& name,
            const math::Vector& position,
            const math::Vector& base_point,
            float rotation,
            const std::vector<math::Vector>& vertices,
            const std::string& texture_name) const;

    private:

        Editor* m_editor;
    };
}
