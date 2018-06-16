
#pragma once

#include "ObjectProxies/IObjectProxy.h"
#include "EntityRepository.h"

#include <string>

namespace editor
{
    class Editor;

    class ObjectFactory
    {
    public:

        ObjectFactory(Editor* editor);

        IObjectProxyPtr CreateObject(const char* object_name) const;

        IObjectProxyPtr CreateEntity(const char* entity_name) const;
        IObjectProxyPtr CreatePath(const std::string& name, const std::vector<math::Vector>& points) const;
        IObjectProxyPtr CreatePrefab(const std::string& prefab_name) const;
        IObjectProxyPtr CreatePolygon(
            const math::Vector& position,
            const math::Vector& base_point,
            float rotation,
            const std::vector<math::Vector>& vertices,
            const std::string& texture_name) const;

    private:

        Editor* m_editor;
        EntityRepository m_repository;
    };
}
