#pragma once
#include "ECS/Commands.hpp"

namespace worse
{

    class Engine
    {
    public:
        static void initialize(ecs::Commands commands);
        static void tick(ecs::Commands commands);
        static void shutdown();
    };

} // namespace worse