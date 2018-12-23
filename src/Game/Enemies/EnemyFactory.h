
#pragma once

#include "IEnemyFactory.h"

namespace game
{
    class DamageController;

    class EnemyFactory : public IEnemyFactory
    {
    public:

        EnemyFactory(mono::EventHandler& event_handler, game::DamageController& damage_controller);
    
        EnemyPtr CreateCacoDemon(const math::Vector& position) override;
        EnemyPtr CreateInvader(const math::Vector& position) override;
        EnemyPtr CreatePathInvader(mono::IPathPtr& path) override;
        EnemyPtr CreatePathInvader(const math::Vector& position, const std::vector<Attribute>& attributes);
        EnemyPtr CreateBlackSquare(const math::Vector& position, float trigger_distance) override;
        EnemyPtr CreateBeast(const math::Vector& position) override;
        EnemyPtr CreateBat(const math::Vector& position) override;

        EnemyPtr CreateFromName(
            const char* name, const math::Vector& position, const std::vector<Attribute>& attributes) override;
        
    private:

        mono::EventHandler& m_event_handler;
        game::DamageController& m_damage_controller;
    };
}
