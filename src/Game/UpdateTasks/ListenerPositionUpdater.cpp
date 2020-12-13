
#include "ListenerPositionUpdater.h"
#include "Audio/AudioSystem.h"
#include "Math/Vector.h"
#include "AIKnowledge.h"

using namespace game;

void ListenerPositionUpdater::Update(const mono::UpdateContext& update_context)
{
    if(game::g_player_one.player_state != game::PlayerState::ALIVE)
        return;

    mono::ListenerPosition(game::g_player_one.position.x, game::g_player_one.position.y);
    mono::ListenerVelocity(game::g_player_one.velocity.x, game::g_player_one.velocity.y);
    mono::ListenerDirection(game::g_player_one.direction);
}
