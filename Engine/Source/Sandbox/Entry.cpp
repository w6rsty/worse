#include "Log.hpp"
#include "Engine.hpp"
#include "Profiling/Stopwatch.hpp"
#include "Renderer.hpp"

#include "ECS/Registry.hpp"
#include "ECS/Schedule.hpp"

#include "World.hpp"

#include <cstring>

using namespace worse;

int main()
{
    Logger::initialize();
    ecs::Registry registry;
    ecs::Schedule schedule;

    schedule.addSystem<ecs::CoreStage::StartUp, &Engine::initialize>();
    schedule.addSystem<ecs::CoreStage::StartUp, &Renderer::initialize>();
    schedule.addSystem<ecs::CoreStage::StartUp, &ImGuiRenderer::initialize>();
    schedule.addSystem<ecs::CoreStage::StartUp, &World::initialize>();
    schedule.addSystem<ecs::CoreStage::StartUp, &World::setupScene>();
    schedule.addSystem<ecs::CoreStage::StartUp, buildMeshes>();
    schedule.addSystem<ecs::CoreStage::StartUp, buildMaterials>();

    schedule.addSystem<ecs::CoreStage::Update, &Engine::tick>();
    schedule.addSystem<ecs::CoreStage::Update, &World::updateInput>();
    schedule.addSystem<ecs::CoreStage::Update, &World::update>();
    schedule.addSystem<ecs::CoreStage::Update, buildDrawcalls>();
    schedule.addSystem<ecs::CoreStage::Update, &ImGuiRenderer::tick>();
    schedule.addSystem<ecs::CoreStage::Update, &Renderer::tick>();

    schedule.addSystem<ecs::CoreStage::CleanUp, &ImGuiRenderer::shutdown>();
    schedule.addSystem<ecs::CoreStage::CleanUp, &Renderer::shutdown>();
    schedule.addSystem<ecs::CoreStage::CleanUp, &Engine::shutdown>();

    profiling::Stopwatch mainTimer;
    schedule.initialize(registry);
    WS_LOG_INFO("Main",
                "ECS schedule initialized in {} ms",
                mainTimer.elapsedMs());

    while (!Window::shouldClose())
    {
        schedule.run(registry);
    }
    schedule.shutdown(registry);

    Logger::shutdown();
}