#pragma once
#include "ECS/Commands.hpp"
#include "ECS/Resource.hpp"
#include "Prefab.hpp"

namespace worse
{

    class ImGuiRenderer
    {
    public:
        static void initialize();
        static void shutdown();
        static void tick(ecs::Commands commands,
                         ecs::Resource<GlobalContext> globalContext);
    };

} // namespace worse