
#include "ListenerPositionUpdater.h"
#include "Audio/AudioListener.h"
#include "Math/Vector.h"
#include "AIKnowledge.h"

using namespace game;

void ListenerPositionUpdater::doUpdate(unsigned int delta)
{
    mono::ListenerPosition(game::player_one.position.x, game::player_one.position.y);
}
