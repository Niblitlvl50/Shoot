
#pragma once

namespace game
{
    //
    // http://gizma.com/easing/
    // A bunch of easing functions for movement.
    //
    // b: start value
    // c: change in value
    //

    inline float EaseInCubic(float time, float duration, float b, float c)
    {
        time /= duration;
        return c * time * time * time + b;
    };
    
    inline float EaseOutCubic(float time, float duration, float b, float c)
    {
        time /= duration;
        time -= 1.0f;
        return c * (time * time * time + 1.0f) + b;
    };

    inline float EaseInOutCubic(float time, float duration, float b, float c)
    {
        time /= duration / 2.0f;
        if(time < 1.0f)
            return c / 2.0f * time * time * time + b;
        
        time -= 2.0f;
        return c / 2.0f * (time * time * time + 2.0f) + b;
    };

    using EaseFunction = float (*)(float time, float duration, float start_value, float end_value);    
}
