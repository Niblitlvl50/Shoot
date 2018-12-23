
#pragma once

#include "MonoFwd.h"
#include "MonoPtrFwd.h"
#include "Math/MathFwd.h"
#include <memory>
#include <vector>

struct Attribute;

namespace game
{
    using EnemyPtr = std::shared_ptr<class Enemy>;

    class IEnemyFactory
    {
    public:
        virtual ~IEnemyFactory()
        { }

        virtual EnemyPtr CreateCacoDemon(const math::Vector& position) = 0;
        virtual EnemyPtr CreateInvader(const math::Vector& position) = 0;
        virtual EnemyPtr CreatePathInvader(mono::IPathPtr& path) = 0;
        virtual EnemyPtr CreateBlackSquare(const math::Vector& position, float trigger_distance) = 0;
        virtual EnemyPtr CreateBeast(const math::Vector& position) = 0;
        virtual EnemyPtr CreateBat(const math::Vector& position) = 0;

        virtual EnemyPtr CreateFromName(
            const char* name, const math::Vector& position, const std::vector<Attribute>& attributes) = 0;
    };
}
