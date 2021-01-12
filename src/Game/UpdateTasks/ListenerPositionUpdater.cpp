
#include "ListenerPositionUpdater.h"
#include "AIKnowledge.h"

void game::ListenerPositionUpdater::Update(const mono::UpdateContext& update_context)
{
    const game::PlayerInfo& first_player = game::g_players[0];
    if(first_player.player_state != game::PlayerState::ALIVE)
        return;

    //mono::ListenerPosition(first_player.position.x, first_player.position.y);
    //mono::ListenerVelocity(first_player.velocity.x, first_player.velocity.y);
    //mono::ListenerDirection(first_player.direction);
}
