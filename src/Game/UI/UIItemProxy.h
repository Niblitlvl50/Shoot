
#pragma once

#include "MonoFwd.h"
#include "Math/MathFwd.h"
#include "UI/UISystem.h"

#include <cstdint>

namespace game
{
    class UIItemProxy
    {
    public:

        UIItemProxy(
            class UISystem* ui_system, mono::TransformSystem* transform_system, mono::IEntityManager* entity_manager);
        ~UIItemProxy();

        void UpdateUIItem(
            const math::Matrix& transform, const math::Quad& bb, uint32_t trigger_hash, const UINavigationSetup& navigation_setup);
        void SetItemState(UIItemState new_state);
        void SetItemCallback(const UIItemCallback& item_callback);
        void ReleaseItemCallback();

        uint32_t GetEntityId() const;

    private:

        UISystem* m_ui_system;
        mono::TransformSystem* m_transform_system;
        mono::IEntityManager* m_entity_manager;
        uint32_t m_entity_id;
        uint32_t m_callback_handle;
    };
}
