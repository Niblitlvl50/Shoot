
#include "PackageLogic.h"
#include "Player/PlayerInfo.h"

using namespace game;

PackageLogic::PackageLogic(uint32_t entity_id, game::PackageInfo* package_info, mono::SystemContext* system_context)
    : m_entity_id(entity_id)
    , m_package_info(package_info)
{

}

void PackageLogic::Update(const mono::UpdateContext& update_context)
{

}
