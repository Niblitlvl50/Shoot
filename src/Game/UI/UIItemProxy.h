
#pragma once

#include "MonoFwd.h"
#include "Math/MathFwd.h"

#include <cstdint>

namespace game
{
    class UIItemProxy
    {
    public:

        UIItemProxy(
            class UISystem* ui_system, mono::TransformSystem* transform_system, mono::IEntityManager* entity_manager);
        ~UIItemProxy();

        void UpdateUIItem(const math::Matrix& transform, const math::Quad& bb);

    private:

        UISystem* m_ui_system;
        mono::TransformSystem* m_transform_system;
        mono::IEntityManager* m_entity_manager;
        uint32_t m_entity_id;
    };
}
