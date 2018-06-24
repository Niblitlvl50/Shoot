
#include "GameObjectFactory.h"
#include "DamageController.h"
#include "Barrel.h"
#include "Explosion.h"
#include "Player/Shuttle.h"
#include "RenderLayers.h"
#include "Events/SpawnEntityEvent.h"
#include "Events/ShockwaveEvent.h"

#include <EventHandler/EventHandler.h>
#include <cstring>
#include <functional>

namespace
{
    void RedBarrelDestroyed(unsigned int rectord_id, mono::EventHandler& event_handler, mono::IEntityPtr barrel)
    {
        game::ExplosionConfiguration explosion_config;
        explosion_config.position = barrel->Position();
        explosion_config.scale = 2.0f;
        explosion_config.rotation = 0.0f;
        explosion_config.sprite_file = "res/sprites/explosion.sprite";

        const game::SpawnEntityEvent spawn_event(
            std::make_shared<game::Explosion>(explosion_config, event_handler), game::LayerId::FOREGROUND);
        event_handler.DispatchEvent(spawn_event);
        event_handler.DispatchEvent(game::ShockwaveEvent(explosion_config.position, 150));
    }
}

using namespace game;

GameObjectFactory::GameObjectFactory(mono::EventHandler& event_handler, game::DamageController& damage_controller)
    : m_event_handler(event_handler)
    , m_damage_controller(damage_controller)
{ }

mono::IPhysicsEntityPtr GameObjectFactory::CreateShuttle(
    const math::Vector& position, const System::ControllerState& gamepad, DestroyedFunction callback) const
{
    auto shuttle = std::make_shared<Shuttle>(position, m_event_handler, gamepad);
    m_damage_controller.CreateRecord(shuttle->Id(), callback);

    return shuttle;
}

mono::IPhysicsEntityPtr GameObjectFactory::CreateGameObject(const char* name, const std::vector<Attribute>& attributes) const
{
    if(std::strcmp(name, "barrel_red") == 0)
    {
        Barrel::Configuration config;
        config.sprite_file = "res/sprites/barrel_red.sprite";
        config.scale = math::Vector(0.6f, 1.0f);
        config.health = 100;

        auto red_barrel = std::make_shared<Barrel>(config);
        auto callback_func = std::bind(RedBarrelDestroyed, std::placeholders::_1, std::ref(m_event_handler), red_barrel);

        m_damage_controller.CreateRecord(red_barrel->Id(), callback_func);

        return red_barrel;
    }
    else if(std::strcmp(name, "barrel_green") == 0)
    {
        Barrel::Configuration config;
        config.sprite_file = "res/sprites/barrel_green.sprite";
        config.scale = math::Vector(0.6f, 1.0f);
        config.health = -1;

        return std::make_shared<Barrel>(config);
    }

    return nullptr;
}
