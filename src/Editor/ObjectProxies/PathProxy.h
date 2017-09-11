
#pragma once

#include "IObjectProxy.h"
#include <memory>

namespace editor
{
    class PathEntity;
    class Editor;

    class PathProxy : public editor::IObjectProxy
    {
    public:

        PathProxy(const std::shared_ptr<PathEntity>& path, Editor* editor);

        virtual const char* Name() const;
        virtual unsigned int Id() const;
        virtual mono::IEntityPtr Entity();
        virtual void SetSelected(bool selected);
        virtual bool Intersects(const math::Vector& position) const;
        virtual std::vector<Grabber> GetGrabbers() const;
        virtual std::vector<SnapPoint> GetSnappers() const;
        virtual void UpdateUIContext(UIContext& context);
        virtual std::vector<ID_Attribute> GetAttributes() const;
        virtual void SetAttributes(const std::vector<ID_Attribute>& attributes);
        virtual void Visit(IObjectVisitor& visitor);
        
        std::shared_ptr<PathEntity> m_path;
        Editor* m_editor;
    };
}
