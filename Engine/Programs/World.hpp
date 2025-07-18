#pragma once
#include "Log.hpp"
#include "Window.hpp"
#include "Event.hpp"
#include "Profiling/Stopwatch.hpp"

#include "ImGuiRenderer.hpp"
#include "Camera.hpp"
#include "Mesh.hpp"
#include "Material.hpp"
#include "AssetServer.hpp"

#include "ECS/QueryView.hpp"
#include "ECS/Resource.hpp"

#include "PointCloud/Cloud.hpp"

#include <string>
#include <vector>
#include <unordered_map>

using namespace worse;

enum class ApplicationState
{
    Undefined,
    Begin,
    Running,
    End,
};

class World
{
public:
    inline static float cameraMoveSpeed = 1.0f;

    // 相机引用，在initialize中设置，在UI中使用
    inline static Camera* currentCamera = nullptr;

    // 点云相关的静态成员
    inline static pc::Cloud pointCloudData;
    inline static ecs::Entity pointCloudEntity = ecs::Entity::null();
    inline static bool hasPointCloud           = false;

    // 运行时加载的点云网格映射 - 文件名到网格索引的映射
    inline static std::unordered_map<std::string, size_t> loadedMeshes;
    inline static std::vector<std::string> availableFiles;

    // 加载进度相关
    inline static bool isLoadingMesh             = false;
    inline static std::string currentLoadingFile = "";
    inline static float loadingProgress          = 0.0f;

    // PCL处理相关
    inline static bool isProcessingWithPCL          = false;
    inline static std::string currentProcessingFile = "";
    inline static float pclProcessingProgress       = 0.0f;

    // 电力线参数弹出窗口相关
    struct PowerLineParameter
    {
        int id;                          // 线路ID
        std::string name;                // 线路名称 (UUID)
        float start_x, start_y, start_z; // 起点坐标 (m)
        float end_x, end_y, end_z;       // 终点坐标 (m)
        float length;                    // 线路长度 (m)
        float width;                     // 线路宽度/直径 (mm)
        float max_sag;                   // 最大弧垂 (m)
        float catenary_a;                // 悬链线方程系数a
        float catenary_h;                // 悬链线方程水平位移h
        float catenary_k;                // 悬链线方程垂直位移k
    };

    inline static bool showPowerLineParamsPopup = false;
    inline static std::vector<PowerLineParameter> powerLineParams;
    inline static std::string lastProcessedFile = "";

    // 当前活跃的点云文件名
    inline static std::string currentActiveFile = "";

    // 固定的点云文件目录
    inline static const std::string POINT_CLOUD_DIRECTORY =
        "/Users/w6rsty/dev/Cpp/worse_pc/Engine/Binary/LAS/";

    // 视角相关的静态变量
    static constexpr math::Vector3 defaultCameraPosition =
        math::Vector3{0.0f, 0.0f, 5.0f};
    // 顶视图相机位置
    static constexpr math::Vector3 downwardCameraPosition =
        math::Vector3{0.0f, 10.0f, 0.0f};
    // 侧视图相机位置
    static constexpr math::Vector3 sideCameraPosition =
        math::Vector3{10.0f, 0.0f, 0.0f};

    // 相机跟随速度
    static constexpr float cameraFollowSpeed = 5.0f;
    // 相机朝向
    inline static math::Quaternion cameraOrientation =
        math::Quaternion::IDENTITY();

    inline static math::Vector3 pointCloudCenter =
        math::Vector3{0.0f, 0.0f, 0.0f};
    inline static float pointCloudBoundingRadius = 5.0f; // 视角切换辅助函数

    inline static std::size_t defaultPointMaterialIndex = 0;

    static void resetCameraView(Camera* camera);
    static void setCameraToPerspectiveView(Camera* camera);
    static void setCameraToOrthographicView(Camera* camera);
    static void setCameraToTopView(Camera* camera);
    static void setCameraToSideView(Camera* camera);
    static void fitCameraToPointCloud(Camera* camera);

    // 初始化可用文件列表的函数
    static void initializeAvailableFiles();
    // 运行时加载点云网格的函数
    static bool loadPointCloudMesh(const std::string& filename,
                                   ecs::Commands& commands);
    // 清理所有已加载的网格的函数
    static void clearAllLoadedMeshes(ecs::Commands& commands);
    // PCL处理相关函数
    static bool processMesh(const std::string& filename,
                            ecs::Commands& commands);
    // 通过文件名切换点云Entity的函数（支持运行时加载）
    static bool switchToPointCloud(const std::string& filename,
                                   ecs::Commands& commands);

    // 页面布局声明
    static void editorLayout(ecs::Commands commands,
                             ecs::Resource<GlobalContext> context);

    static void initialize(ecs::Commands commands)
    {
        // clang-format off

        // 初始化相机
        {
            Camera& camera = commands.emplaceResource<Camera>();
            camera.setAspectRatio(
                static_cast<float>(Window::getWidth()) /
                static_cast<float>(Window::getHeight())
            );
            camera.setPosition(defaultCameraPosition);

            EventBus::subscribe(EventType::WindowResized,
            [&camera](Event const& payload)
            {
                camera.setAspectRatio(
                    static_cast<float>(Window::getWidth()) /
                    static_cast<float>(Window::getHeight())
                );
            });
    
            // 设置camera引用供UI使用
            currentCamera = &camera;
        }
        
        // 注册页面路由
        PageRouter<ApplicationState>& router =  ImGuiRenderer::registerStates<ApplicationState>(commands, ApplicationState::Running);
        router.registerPage(ApplicationState::Running, editorLayout);
        // clang-format on
    }

    // clang-format off
    static void setupScene(
        ecs::Commands commands,
        ecs::Resource<Camera> camera,
        ecs::ResourceArray<StandardMaterial> materials,
        ecs::Resource<AssetServer> assetServer,
        ecs::ResourceArray<Mesh> meshes
    )
    {
        defaultPointMaterialIndex = materials->add(StandardMaterial{
            .albedo = math::Vector4{1.0f, 1.0f, 1.0f, 1.0f},
        });

        assetServer->load();

        // dummy floor , prevent null drawcall
        commands.spawn(
            LocalTransform{.position = math::Vector3{0.0f, -100.0f, 0.0f}},
            Mesh3D{meshes.add(Cube{})},
            MeshMaterial{materials->add(StandardMaterial{})}
        );

        // 初始化可用文件列表（不预加载网格）
        initializeAvailableFiles();

        WS_LOG_INFO("Scene", "Scene setup completed. {} point cloud files found. Use the UI to load and select point clouds.", availableFiles.size());
    }
    // clang-format on

    static void updateInput(ecs::Resource<Camera> camera,
                            ecs::Resource<GlobalContext> globalContext);

    // clang-format off
    static void update(
        ecs::Commands commands,
        ecs::QueryView<LocalTransform> view,
        ecs::Resource<GlobalContext> globalContext,
        ecs::Resource<Camera> camera
    )
    {
        static profiling::Stopwatch frameTimer;
        globalContext->deltaTime = frameTimer.elapsedSec();
        globalContext->time += globalContext->deltaTime;
        frameTimer.reset();
    }
    // clang-format on
};