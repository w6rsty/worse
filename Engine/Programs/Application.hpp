#pragma once

#include "ECS/Registry.hpp"
#include "ECS/Schedule.hpp"

namespace worse::pc
{

    class Application
    {
    public:
        Application(int argc, char** argv);
        ~Application();

        void run();

    private:
        ecs::Registry m_registry;
        ecs::Schedule m_schedule;
    };

} // namespace worse::pc