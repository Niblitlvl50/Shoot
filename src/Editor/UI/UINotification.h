
#pragma once

#include <string>

namespace editor
{
    struct Notification
    {
        Notification()
        { }

        Notification(const std::string& icon, const std::string& text, float time_left_s)
            : icon(icon)
            , text(text)
            , time_left_s(time_left_s)
        { }

        std::string icon;
        std::string text;
        float time_left_s;
    };
}
