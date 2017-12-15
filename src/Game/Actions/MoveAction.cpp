
#include "MoveAction.h"
#include "Entity/IEntity.h"
#include <algorithm>

void game::UpdateMoveContexts(unsigned int delta, std::vector<MoveActionContext>& move_contexts)
{
    for(MoveActionContext& move_context : move_contexts)
    {
        move_context.counter += delta;
        move_context.counter = std::min(move_context.counter, move_context.duration);

        const math::Vector& delta = move_context.end_position - move_context.start_position;

        const math::Vector new_position(
            move_context.ease_func(float(move_context.counter), float(move_context.duration), move_context.start_position.x, delta.x),        
            move_context.ease_func(float(move_context.counter), float(move_context.duration), move_context.start_position.y, delta.y)
        );

        move_context.entity->SetPosition(new_position);
    }

    const auto remove_func = [](const MoveActionContext& move_context) {
        return move_context.counter >= move_context.duration;
    };

    for(MoveActionContext& context : move_contexts)
    {
        if(remove_func(context))
        {
            if(context.ping_pong)
            {
                std::swap(context.start_position, context.end_position);
                context.counter = 0;
            }
            else if(context.callback)
            {
                context.callback();
            }
        }
    }

    const auto it = std::remove_if(move_contexts.begin(), move_contexts.end(), remove_func);
    move_contexts.erase(it, move_contexts.end());
}
