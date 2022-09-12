
#pragma once

#include "MonoFwd.h"
#include "Math/Vector.h"
#include "StateMachine.h"
#include "Entity/IEntityLogic.h"

namespace game
{
    struct PackageInfo;

    class PackageLogic : public IEntityLogic
    {
    public:

        PackageLogic(uint32_t entity_id, game::PackageInfo* package_info, mono::SystemContext* system_context);
        void Update(const mono::UpdateContext& update_context) override;

        const uint32_t m_entity_id;
        game::PackageInfo* m_package_info;
    };
}
