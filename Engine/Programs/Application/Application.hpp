#pragma once
#include "ECS/Registry.hpp"
#include "ECS/Schedule.hpp"

class Application
{
public:
    Application(int argc, char** argv);
    ~Application();

    void run();

private:
    worse::ecs::Registry m_registry;
    worse::ecs::Schedule m_schedule;
};
