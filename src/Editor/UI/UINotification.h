
#pragma once

#include <string>

namespace editor
{
    struct Notification
    {
        Notification()
        { }

        Notification(const std::string& icon, const std::string& text, int time_left)
            : icon(icon),
              text(text),
              time_left(time_left)
        { }

        std::string icon;
        std::string text;
        int time_left;
    };
}
