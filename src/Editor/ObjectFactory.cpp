
#include "ObjectFactory.h"

#include "ObjectProxies/PathProxy.h"
#include "Objects/Path.h"

#include "Math/Matrix.h"

using namespace editor;

ObjectFactory::ObjectFactory(Editor* editor)
    : m_editor(editor)
{ }

IObjectProxyPtr ObjectFactory::CreatePath(const std::string& name, const std::vector<math::Vector>& points) const
{
    auto path = std::make_unique<editor::PathEntity>(name, points);
    return std::make_unique<PathProxy>(std::move(path), m_editor);
}
