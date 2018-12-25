
#include "MoveAction.h"
#include "Entity/IEntity.h"
#include "Util/Algorithm.h"
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

    const auto remove_func = [](MoveActionContext& context) {
        bool is_done = context.counter >= context.duration;
        if(is_done)
        {
            if(context.ping_pong)
            {
                std::swap(context.start_position, context.end_position);
                context.counter = 0;
                is_done = false;
            }
            else if(context.callback)
            {
                context.callback();
            }
        }

        return is_done;
    };

    mono::remove_if(move_contexts, remove_func);
}

void game::UpdateRotateContexts(unsigned int delta, std::vector<RotateActionContext>& rotate_contexts)
{
    for(RotateActionContext& rotate_context : rotate_contexts)
    {
        rotate_context.counter += delta;
        rotate_context.counter = std::min(rotate_context.counter, rotate_context.duration);

        //const float intervall = rotate_context.max_rotation - rotate_context.min_rotation;

        const float delta = rotate_context.max_rotation - rotate_context.min_rotation;
        const float new_rotation =
            rotate_context.ease_function(float(rotate_context.counter), float(rotate_context.duration), rotate_context.start_rotation, delta);

        rotate_context.entity->SetRotation(new_rotation);
    }

    const auto remove_func = [](RotateActionContext& context) {

        bool done = (context.counter >= context.duration);

        if(done && context.ping_pong)
        {
            std::swap(context.min_rotation, context.max_rotation);
            context.counter = 0;
            done = false;
        }

        return done;
    };

    mono::remove_if(rotate_contexts, remove_func);
}
