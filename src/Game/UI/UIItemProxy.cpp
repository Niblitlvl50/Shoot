
#include "UIItemProxy.h"
#include "UI/UISystem.h"
#include "Entity/Component.h"

#include "EntitySystem/IEntityManager.h"
#include "TransformSystem/TransformSystem.h"

using namespace game;

UIItemProxy::UIItemProxy(UISystem* ui_system, mono::TransformSystem* transform_system, mono::IEntityManager* entity_manager)
    : m_ui_system(ui_system)
    , m_transform_system(transform_system)
    , m_entity_manager(entity_manager)
{
    const mono::Entity entity = m_entity_manager->CreateEntity("UIItemProxy", { TRANSFORM_COMPONENT, UI_ITEM_COMPONENT });
    m_entity_id = entity.id;
}

UIItemProxy::~UIItemProxy()
{
    m_entity_manager->ReleaseEntity(m_entity_id);
    m_entity_id = mono::INVALID_ID;
}

void UIItemProxy::UpdateUIItem(const math::Matrix& transform, const math::Quad& bb)
{
    m_transform_system->SetTransform(m_entity_id, transform);
    m_transform_system->SetBoundingBox(m_entity_id, bb);

    UINavigationSetup navigation_setup;
    m_ui_system->UpdateUIItem(m_entity_id, 0, 0, game::UIItemState::Enabled, navigation_setup);
}
