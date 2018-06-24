
#pragma once

#include "IGameObjectFactory.h"
#include <MonoFwd.h>

namespace game
{
    class DamageController;

    class GameObjectFactory : public IGameObjectFactory
    {
    public:

        GameObjectFactory(mono::EventHandler& event_handler, game::DamageController& damage_controller);

        mono::IPhysicsEntityPtr CreateShuttle(
            const math::Vector& position, const System::ControllerState& gamepad, DestroyedFunction callback) const override;
        mono::IPhysicsEntityPtr CreateGameObject(const char* name, const std::vector<Attribute>& attributes) const override;

    private:
        mono::EventHandler& m_event_handler;
        game::DamageController& m_damage_controller;
    };
}
