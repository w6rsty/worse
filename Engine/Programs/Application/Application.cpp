#include "Profiling/Stopwatch.hpp"

#include "Window.hpp"
#include "Engine.hpp"
#include "Renderer.hpp"
#include "ImGuiRenderer.hpp"

#include "World.hpp"
#include "Application.hpp"

Application::Application(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    // clang-format off
        m_schedule.addSystem<ecs::CoreStage::StartUp, Engine::initialize>();
        m_schedule.addSystem<ecs::CoreStage::StartUp, Renderer::initialize>();
        m_schedule.addSystem<ecs::CoreStage::StartUp, ImGuiRenderer::initialize>();
        m_schedule.addSystem<ecs::CoreStage::StartUp, World::initialize>();
        m_schedule.addSystem<ecs::CoreStage::StartUp, World::setupScene>();
        m_schedule.addSystem<ecs::CoreStage::StartUp, buildMaterials>();

        m_schedule.addSystem<ecs::CoreStage::Update, Engine::tick>();
        m_schedule.addSystem<ecs::CoreStage::Update, World::updateInput>();
        m_schedule.addSystem<ecs::CoreStage::Update, World::update>();
        m_schedule.addSystem<ecs::CoreStage::Update, buildDrawcalls>();
        m_schedule.addSystem<ecs::CoreStage::Update, ImGuiRenderer::tick>();
        m_schedule.addSystem<ecs::CoreStage::Update, Renderer::tick>();

        m_schedule.addSystem<ecs::CoreStage::CleanUp, World::clearAllLoadedMeshes>();
        m_schedule.addSystem<ecs::CoreStage::CleanUp, ImGuiRenderer::shutdown>();
        m_schedule.addSystem<ecs::CoreStage::CleanUp, Renderer::shutdown>();
        m_schedule.addSystem<ecs::CoreStage::CleanUp, Engine::shutdown>();
    // clang-format on

    {
        profiling::Stopwatch timer;
        m_schedule.initialize(m_registry);
        WS_LOG_INFO("Application",
                    "Initialization took {} ms",
                    timer.elapsedMs());
    }
}

Application::~Application()
{
    m_schedule.shutdown(m_registry);
}

void Application::run()
{
    while (!Window::shouldClose())
    {
        m_schedule.run(m_registry);
    }
}
