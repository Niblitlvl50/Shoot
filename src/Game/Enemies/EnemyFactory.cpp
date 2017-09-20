
#include "EnemyFactory.h"
#include "Enemy.h"
#include "Paths/IPath.h"
#include "Paths/PathFactory.h"

#include "CacoDemonController.h"
#include "InvaderController.h"
#include "BlackSquareController.h"

#include "DefinedAttributes.h"

#include <cstring>
#include <string>

using namespace game;

EnemyFactory::EnemyFactory(mono::EventHandler& event_handler)
    : m_eventHandler(event_handler)
{ }

game::EnemyPtr EnemyFactory::CreateCacoDemon(const math::Vector& position)
{
    EnemySetup setup;
    setup.sprite_file = "res/sprites/cacodemon.sprite";
    setup.size = 2.0f;
    setup.mass = 500.0f;
    setup.position = position;
    setup.controller = std::make_unique<CacoDemonController>(m_eventHandler);

    return std::make_shared<game::Enemy>(setup);
}

game::EnemyPtr EnemyFactory::CreateInvader(const math::Vector& position)
{
    EnemySetup setup;
    setup.sprite_file = "res/sprites/invader.sprite";
    setup.size = 1.0f;
    setup.mass = 50.0f;
    setup.position = position;
    setup.controller = std::make_unique<InvaderController>(m_eventHandler);

    return std::make_shared<game::Enemy>(setup);
}

game::EnemyPtr EnemyFactory::CreatePathInvader(const mono::IPathPtr& path)
{
    EnemySetup setup;
    setup.sprite_file = "res/sprites/invader.sprite";
    setup.size = 1.0f;
    setup.mass = 50.0f;
    setup.position = path->GetGlobalPosition() + path->GetPositionByLength(0.0f);
    setup.controller = std::make_unique<InvaderPathController>(path, m_eventHandler);

    return std::make_shared<game::Enemy>(setup);
}

game::EnemyPtr EnemyFactory::CreatePathInvader(const math::Vector& position, const std::vector<ID_Attribute>& attributes)
{
    const char* filename = nullptr;
    const bool found_filename = world::FindAttribute(world::FILEPATH_ATTRIBUTE, attributes, filename);

    if(found_filename)
    {
        const std::string& full_filename = std::string("res/paths/") + filename + ".path";
        return CreatePathInvader(mono::CreatePath(full_filename.c_str()));
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
    setup.controller = std::make_unique<BlackSquareController>(trigger_distance, m_eventHandler);

    return std::make_shared<game::Enemy>(setup);
}

game::EnemyPtr EnemyFactory::CreateFromName(
    const char* name, const math::Vector& position, const std::vector<ID_Attribute>& attributes)
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

    return nullptr;
}
