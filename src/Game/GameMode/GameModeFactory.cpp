
#include "GameModeFactory.h"
#include "Zones/ZoneFlow.h"
#include "PacketDeliveryGameMode.h"

#include "System/Hash.h"

namespace
{
    class NullGameMode : public game::IGameMode
    {
    public:

        void Begin(
            mono::IZone* zone,
            mono::IRenderer* renderer,
            mono::SystemContext* system_context,
            mono::EventHandler* event_handler,
            const game::LevelMetadata& level_metadata) override { }
        int End(mono::IZone* zone) override { return game::ZoneResult::ZR_COMPLETED; }
        void Update(const mono::UpdateContext& update_context) override {}
    };
}

game::IGameModePtr game::GameModeFactory::CreateGameMode(uint32_t game_mode)
{
    if(game_mode == hash::Hash("package_delivery_mode"))
        return std::make_unique<PacketDeliveryGameMode>();

    return std::make_unique<NullGameMode>();
}

std::vector<std::string> game::GameModeFactory::GetAllGameModes()
{
    return {
        "none",
        "package_delivery_mode",
        "capture_the_flag"
    };
}
