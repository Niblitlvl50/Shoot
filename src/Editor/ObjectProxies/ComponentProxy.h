
#pragma once

#include "IObjectProxy.h"
#include <vector>
#include <string>

struct Component;
class IEntityManager;

namespace editor
{
    class ComponentProxy : public IObjectProxy
    {
    public:

        ComponentProxy(uint32_t entity_id, const std::string& name, IEntityManager* entity_manager);
        ComponentProxy(uint32_t entity_id, const std::string& name, const std::vector<Component>& components, IEntityManager* entity_manager);
        ~ComponentProxy();
    
        const char* Name() const override;
        uint32_t Id() const override;
        mono::IEntityPtr Entity() override;

        void SetSelected(bool selected) override;
        bool Intersects(const math::Vector& position) const override;
        std::vector<struct Grabber> GetGrabbers() const override;
        std::vector<SnapPoint> GetSnappers() const override;

        void UpdateUIContext(struct UIContext& context) override;

        const std::vector<Component>& GetComponents() const override;
        std::vector<Component>& GetComponents() override;

        std::unique_ptr<IObjectProxy> Clone() const override;
        void Visit(class IObjectVisitor& visitor) override;

    private:

        const uint32_t m_entity_id;
        std::string m_name;
        std::vector<Component> m_components;
        IEntityManager* m_entity_manager;
    };
}
