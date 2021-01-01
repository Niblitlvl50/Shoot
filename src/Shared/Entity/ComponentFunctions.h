
#pragma once

namespace mono
{
    class IEntityManager;
}

namespace shared
{
    void RegisterSharedComponents(mono::IEntityManager* entity_manager);
}
