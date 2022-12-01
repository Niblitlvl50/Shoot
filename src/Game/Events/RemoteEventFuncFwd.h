
#pragma once

#include "EventHandler/EventToken.h"
#include <functional>

namespace game
{
    struct RemoteInputMessage;
    struct ViewportMessage;

    using RemoteInputMessageFunc = std::function<mono::EventResult (const RemoteInputMessage&)>;
    using ViewportMessageFunc = std::function<mono::EventResult (const ViewportMessage&)>;
}
