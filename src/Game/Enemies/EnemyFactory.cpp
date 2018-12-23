
#include "EnemyFactory.h"
#include "Enemy.h"
#include "Paths/IPath.h"
#include "Paths/PathFactory.h"

#include "CacoDemonController.h"
#include "InvaderController.h"
#include "InvaderPathController.h"
#include "BlackSquareController.h"
#include "BeastController.h"
#include "BatController.h"

#include "DefinedAttributes.h"
#include "DamageController.h"

#include <cstring>
#include <string>

using namespace game;

EnemyFactory::EnemyFactory(mono::EventHandler& event_handler, game::DamageController& damage_controller)
    : m_event_handler(event_handler)
    , m_damage_controller(damage_controller)
{ }

game::EnemyPtr EnemyFactory::CreateCacoDemon(const math::Vector& position)
{
    EnemySetup setup;
    setup.sprite_file = "res/sprites/cacodemon.sprite";
    setup.size = 2.0f;
    setup.mass = 500.0f;
    setup.position = position;
    setup.controller = std::make_unique<CacoDemonController>(m_event_handler);

    auto enemy = std::make_shared<game::Enemy>(setup);
    m_damage_controller.CreateRecord(enemy->Id(), nullptr);

    return enemy;
}

game::EnemyPtr EnemyFactory::CreateInvader(const math::Vector& position)
{
    EnemySetup setup;
    setup.sprite_file = "res/sprites/invader.sprite";
    setup.size = 1.0f;
    setup.mass = 50.0f;
    setup.position = position;
    setup.controller = std::make_unique<InvaderController>(m_event_handler);

    auto enemy = std::make_shared<game::Enemy>(setup);
    m_damage_controller.CreateRecord(enemy->Id(), nullptr);

    return enemy;
}

game::EnemyPtr EnemyFactory::CreatePathInvader(mono::IPathPtr& path)
{
    EnemySetup setup;
    setup.sprite_file = "res/sprites/invader.sprite";
    setup.size = 1.0f;
    setup.mass = 50.0f;
    setup.position = path->GetGlobalPosition() + path->GetPositionByLength(0.0f);
    setup.controller = std::make_unique<InvaderPathController>(path, m_event_handler);

    auto enemy = std::make_shared<game::Enemy>(setup);
    m_damage_controller.CreateRecord(enemy->Id(), nullptr);

    return enemy;
}

game::EnemyPtr EnemyFactory::CreatePathInvader(const math::Vector& position, const std::vector<Attribute>& attributes)
{
    const char* filename = nullptr;
    const bool found_filename = world::FindAttribute(world::FILEPATH_ATTRIBUTE, attributes, filename);

    if(found_filename && strlen(filename) != 0)
    {
        const std::string& full_filename = std::string("res/paths/") + filename + ".path";
        mono::IPathPtr path = mono::CreatePath(full_filename.c_str());
        return CreatePathInvader(path);
    }

    return nullptr;
}

game::EnemyPtr EnemyFactory::CreateBlackSquare(const math::Vector& position, float trigger_distance)
{
    EnemySetup setup;
    setup.sprite_file = "res/sprites/invader.sprite";
    setup.size = 1.0f;
    setup.mass = 20.0f;
    setup.position = position;
    setup.controller = std::make_unique<BlackSquareController>(trigger_distance, m_event_handler);

    auto enemy = std::make_shared<game::Enemy>(setup);
    m_damage_controller.CreateRecord(enemy->Id(), nullptr);

    return enemy;
}

game::EnemyPtr EnemyFactory::CreateBeast(const math::Vector& position)
{
    EnemySetup setup;
    setup.sprite_file = "res/sprites/beast.sprite";
    setup.size = 1.0f;
    setup.mass = 20.0f;
    setup.position = position;
    setup.controller = std::make_unique<BeastController>(m_event_handler);

    auto enemy = std::make_shared<game::Enemy>(setup);
    m_damage_controller.CreateRecord(enemy->Id(), nullptr);

    return enemy;
}

game::EnemyPtr EnemyFactory::CreateBat(const math::Vector& position)
{
    EnemySetup setup;
    setup.sprite_file = "res/sprites/bat.sprite";
    setup.size = 1.0f;
    setup.mass = 20.0f;
    setup.position = position;
    setup.controller = std::make_unique<BatController>(m_event_handler);

    auto enemy = std::make_shared<game::Enemy>(setup);
    m_damage_controller.CreateRecord(enemy->Id(), nullptr);

    return enemy;
}

game::EnemyPtr EnemyFactory::CreateFromName(
    const char* name, const math::Vector& position, const std::vector<Attribute>& attributes)
{
    if(std::strcmp(name, "cacodemon") == 0)
        return CreateCacoDemon(position);
    else if(std::strcmp(name, "invader") == 0)
        return CreateInvader(position);
    else if(std::strcmp(name, "pathinvader") == 0)
        return CreatePathInvader(position, attributes);
    else if(std::strcmp(name, "blacksquare") == 0)
    {
        float trigger_distance = 10.0f;
        world::FindAttribute(world::TRIGGER_RADIUS_ATTRIBUTE, attributes, trigger_distance);
        return CreateBlackSquare(position, trigger_distance);
    }
    else if(std::strcmp(name, "beast") == 0)
    {
        return CreateBeast(position);
    }
    else if(std::strcmp(name, "bat") == 0)
    {
        return CreateBat(position);
    }

    return nullptr;
}
