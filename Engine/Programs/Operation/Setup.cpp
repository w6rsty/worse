#include "imgui.h"
#include "Window.hpp"
#include "Event.hpp"
#include "ImGuiRenderer.hpp"
#include "../Application/World.hpp"

void setupCamera(ecs::Commands commands)
{
    Camera& camera = commands.emplaceResource<Camera>();

    camera.setPosition(World::defaultCameraPosition);
    camera.setPerspectiveParams(math::toRadians(60.0f),
                                static_cast<float>(Window::getWidth()) /
                                    static_cast<float>(Window::getHeight()),
                                10.0f,
                                10000.0f);

    EventBus::subscribe(EventType::WindowResized,
                        [&camera](Event const& payload)
                        {
                            camera.setAspectRatio(static_cast<float>(Window::getWidth()) /
                                                  static_cast<float>(Window::getHeight()));
                        });

    // 设置camera引用供UI使用
    World::currentCamera = &camera;
}

void setupLayout(ecs::Commands commands)
{
    LayoutData& layoutData = commands.emplaceResource<LayoutData>();

    layoutData.leftPanelWidth = 250.0f;
    layoutData.leftPanelFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;

    layoutData.rightPanelWidth = 300.0f;
    layoutData.rightPanelFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;

    layoutData.toolbarHeight = 50.0f;
    layoutData.toolbarFlags  = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                              ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;

    layoutData.statusHeight     = 60.0f;
    layoutData.statusPanelFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                  ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;

    // 注册页面路由
    PageRouter<ApplicationState>& router =
        ImGuiRenderer::registerStates<ApplicationState>(commands, ApplicationState::Running);
    router.registerPage(ApplicationState::Running, World::mainLayout);
}

void World::setupScene(ecs::Commands commands, ecs::Resource<Camera> camera,
                       ecs::ResourceArray<StandardMaterial> materials,
                       ecs::Resource<AssetServer> assetServer, ecs::ResourceArray<Mesh> meshes)
{
    defaultMaterial = materials->add(StandardMaterial{});

    assetServer->load();

    // 初始化可用文件列表（不预加载网格）
    initializeLASFiles();

    WS_LOG_INFO("Scene",
                "Scene setup completed. {} point cloud files found. Use the UI "
                "to load and select point clouds.",
                availableFiles.size());
}

void World::initialize(ecs::Commands commands)
{
    setupCamera(commands);
    setupLayout(commands);
}
