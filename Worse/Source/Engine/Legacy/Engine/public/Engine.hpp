#pragma once
#include "ECS/Commands.hpp"

namespace Worse
{

    class Engine
    {
    public:
        static void initialize(ecs::Commands commands);
        static void tick(ecs::Commands commands);
        static void shutdown();
    };

} // namespace Worse