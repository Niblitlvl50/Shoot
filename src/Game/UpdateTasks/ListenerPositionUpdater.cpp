
#include "ListenerPositionUpdater.h"
#include "Audio/AudioSystem.h"
#include "Math/Vector.h"
#include "AIKnowledge.h"

using namespace game;

void ListenerPositionUpdater::Update(const mono::UpdateContext& update_context)
{
    mono::ListenerPosition(game::g_player_one.position.x, game::g_player_one.position.y);
}
