
#pragma once

#include "IEnemyFactory.h"

namespace game
{
    class EnemyFactory : public IEnemyFactory
    {
    public:

        EnemyFactory(mono::EventHandler& event_handler);
    
        EnemyPtr CreateCacoDemon(const math::Vector& position) override;
        EnemyPtr CreateInvader(const math::Vector& position) override;
        EnemyPtr CreatePathInvader(const mono::IPathPtr& path) override;
        EnemyPtr CreatePathInvader(const math::Vector& position, const std::vector<ID_Attribute>& attributes);
        EnemyPtr CreateBlackSquare(const math::Vector& position, float trigger_distance) override;
        EnemyPtr CreateFromName(
            const char* name, const math::Vector& position, const std::vector<ID_Attribute>& attributes) override;
        
    private:

        mono::EventHandler& m_eventHandler;
    };
}
