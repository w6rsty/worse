#pragma once
#include "Camera.hpp"
#include "Mesh.hpp"
#include "Material.hpp"
#include "AssetServer.hpp"

#include "ECS/Commands.hpp"
#include "ECS/Resource.hpp"
#include "Prefab.hpp"

#include "../PointCloud/Cloud.hpp"

#include <string>
#include <vector>
#include <unordered_map>

using namespace worse;

struct LayoutData
{
    bool isVisible = true;

    worse::math::Vector2 viewportSize;

    float leftPanelWidth;
    int leftPanelFlags;

    float rightPanelWidth;
    int rightPanelFlags;

    float toolbarHeight;
    int toolbarFlags;

    float statusHeight;
    int statusPanelFlags;

    bool showPowerLineParamsPopup = false;
};

// 电力线参数弹出窗口相关
struct PowerLineParameter
{
    int id;                       // 线路ID
    std::string name;             // 线路名称 (UUID)
    float startX, startY, startZ; // 起点坐标 (m)
    float endX, endY, endZ;       // 终点坐标 (m)
    float length;                 // 线路长度 (m)
    float width;                  // 线路宽度/直径 (mm)
    float maxSag;                 // 最大弧垂 (m)
    float catenaryA;              // 悬链线方程系数a
    float catenaryH;              // 悬链线方程水平位移h
    float catenaryK;              // 悬链线方程垂直位移k
};

struct CameraData
{
    float moveSpeed;
    float followSpeed;
    math::Quaternion orientation;
};

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
    // 选中的点云
    inline static pc::Cloud cloudData;
    inline static ecs::Entity cloudEntity = ecs::Entity::null();
    inline static bool hasCloud           = false;

    // 运行时加载的点云网格映射 - 文件名到网格索引的映射
    inline static std::unordered_map<std::string, std::size_t> loadedMeshes;
    inline static std::vector<std::string> availableFiles;

    inline static std::vector<PowerLineParameter> powerLineParams;
    inline static std::string lastProcessedFile = "";

    // 当前活跃的点云文件名
    inline static std::string currentActiveFile = "";

    // 固定的点云文件目录
    inline static const std::string POINT_CLOUD_DIRECTORY =
        "/Users/w6rsty/dev/Cpp/worse_pc/Engine/Binary/LAS/";

    inline static std::size_t defaultMaterial = 0;

    static void initializeLASFiles();
    static bool loadCloudMesh(std::string const& filename,
                              ecs::Commands commands);
    static void clearAllLoadedMeshes(ecs::Commands commands);

    static bool processMesh(std::string const& filename,
                            ecs::Commands commands);
    static bool switchToPointCloud(std::string const& filename,
                                   ecs::Commands commands);

    // =========================================================================
    // 布局
    // =========================================================================
    static void mainLayout(ecs::Commands commands,
                           ecs::Resource<GlobalContext> context);
    static void leftPanel(ecs::Commands commands);
    static void rightPanel(ecs::Commands commands);
    static void toolbarPanel(ecs::Commands commands,
                             ecs::Resource<GlobalContext> context);
    static void statusPanel(ecs::Commands commands);

    // =========================================================================
    // 核心
    // =========================================================================

    static void initialize(ecs::Commands commands);

    static void setupScene(ecs::Commands commands, ecs::Resource<Camera> camera,
                           ecs::ResourceArray<StandardMaterial> materials,
                           ecs::Resource<AssetServer> assetServer,
                           ecs::ResourceArray<Mesh> meshes);

    // 控制按键输入
    static void updateInput(ecs::Commands commands, ecs::Resource<GlobalContext> globalContext);

    static void update(ecs::Resource<GlobalContext> globalContext);
    // clang-format on
};