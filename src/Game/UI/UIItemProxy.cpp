
#include "UIItemProxy.h"
#include "UI/UISystem.h"
#include "Entity/Component.h"
#include "System/Hash.h"

#include "EntitySystem/IEntityManager.h"
#include "TransformSystem/TransformSystem.h"

using namespace game;

UIItemProxy::UIItemProxy(UISystem* ui_system, mono::TransformSystem* transform_system, mono::IEntityManager* entity_manager)
    : m_ui_system(ui_system)
    , m_transform_system(transform_system)
    , m_entity_manager(entity_manager)
    , m_callback_handle(mono::INVALID_ID)
{
    const mono::Entity entity = m_entity_manager->CreateEntity("UIItemProxy", { TRANSFORM_COMPONENT, UI_ITEM_COMPONENT });
    m_entity_id = entity.id;
}

UIItemProxy::~UIItemProxy()
{
    ReleaseItemCallback();
    
    m_entity_manager->ReleaseEntity(m_entity_id);
    m_entity_id = mono::INVALID_ID;
}

void UIItemProxy::UpdateUIItem(
    const math::Matrix& transform, const math::Quad& bb, const char* trigger_name, const UINavigationSetup& navigation_setup)
{
    m_transform_system->SetTransform(m_entity_id, transform);
    m_transform_system->SetBoundingBox(m_entity_id, bb);
    m_ui_system->UpdateUIItem(m_entity_id, hash::Hash(trigger_name), 0, game::UIItemState::Enabled, navigation_setup);
}

void UIItemProxy::SetItemCallback(const UIItemCallback& item_callback)
{
    m_callback_handle = m_ui_system->SetUIItemCallback(m_entity_id, item_callback);
}

void UIItemProxy::ReleaseItemCallback()
{
    if(m_callback_handle != mono::INVALID_ID)
        m_ui_system->ReleaseUIItemCallback(m_entity_id, m_callback_handle);

    m_callback_handle = mono::INVALID_ID;
}

uint32_t UIItemProxy::GetEntityId() const
{
    return m_entity_id;
}
