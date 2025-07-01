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

    private:
        static void
        createBasicLayout(ecs::Resource<GlobalContext> globalContext);

        static void createHierarchyPanel();
        static void createPropertiesPanel();
    };

} // namespace worse