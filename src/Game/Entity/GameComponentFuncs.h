
#pragma once

namespace mono
{
    class IEntityManager;
}

namespace game
{
    void RegisterGameComponents(mono::IEntityManager& entity_manager);
}
