#include "Log.hpp"
#include "Event.hpp"
#include "Math/Math.hpp"
#include "Mesh.hpp"
#include "Window.hpp"
#include "Engine.hpp"
#include "Renderer.hpp"
#include "Input/Input.hpp"
#include "Profiling/Stopwatch.hpp"
#include "ImGuiRenderer.hpp"

#include "Material.hpp"
#include "Camera.hpp"
#include "Renderable.hpp"
#include "AssetServer.hpp"

#include "ECS/QueryView.hpp"
#include "ECS/Resource.hpp"
#include "ECS/Registry.hpp"
#include "ECS/Schedule.hpp"

#include <random>

using namespace worse;

struct PlayerTag
{
};

class World
{
public:
    inline static ecs::Entity root   = ecs::Entity::null();
    inline static ecs::Entity player = ecs::Entity::null();

    static void initialize(ecs::Commands commands)
    {
        // clang-format off
        Camera& camera = commands.emplaceResource<Camera>()
            .setPosition(math::Vector3{0.0f, 9.0f, 12.0f})
            .setPerspectiveParams(math::toRadians(60.0f), 1200.0f/720.0f, 0.1f, 500.0f);

        EventBus::subscribe(EventType::WindowResized,
        [&camera](Event const& payload)
        {
            camera.setAspectRatio(
                static_cast<float>(Window::getWidth()) / static_cast<float>(Window::getHeight())
            );
        });
        // clang-format on
    }

    // clang-format off
    static void inputControll(
        ecs::Commands commands,
        ecs::Resource<Camera> camera,
        ecs::Resource<GlobalContext> globalContext
    )
    {
        if (Input::isKeyDown(KeyCode::Escape))
        {
            Window::close();
        }

        // 获取player的位置和变换
        LocalTransform& playerTransform = commands.getComponent<LocalTransform>(player);
        math::Vector3 playerPosition = playerTransform.position;

        float const moveSpeed = 5.0f * globalContext->deltaTime;

        // WASD控制player移动（水平面）
        math::Vector3 moveDirection = math::Vector3::ZERO();

        if (Input::isKey(KeyCode::W) || (Input::getThumbStickLeft().y < 0.0f))
        {
            moveDirection.z -= 1.0f; // 向前（负Z方向）
        }
        if (Input::isKey(KeyCode::S) || (Input::getThumbStickLeft().y > 0.0f))
        {
            moveDirection.z += 1.0f; // 向后（正Z方向）
        }
        if (Input::isKey(KeyCode::A) || (Input::getThumbStickLeft().x < 0.0f))
        {
            moveDirection.x -= 1.0f; // 向左（负X方向）
        }
        if (Input::isKey(KeyCode::D) || (Input::getThumbStickLeft().x > 0.0f))
        {
            moveDirection.x += 1.0f; // 向右（正X方向）
        }

        if (math::length(moveDirection) > 0.0f)
        {
            moveDirection = math::normalize(moveDirection);
            playerPosition += moveDirection * moveSpeed;
        }

        // 垂直移动
        if (Input::isKey(KeyCode::Q))
        {
            playerPosition.y += moveSpeed; // 向上
        }
        if (Input::isKey(KeyCode::E))
        {
            playerPosition.y -= moveSpeed; // 向下
        }

        // 更新 player 位置
        playerTransform.position = playerPosition;

        // 相机追踪设置
        math::Vector3 const cameraOffset = math::Vector3{0.0f, 8.0f, 6.0f}; // 相机相对于 player 的偏移
        float const followSpeed          = 4.0f; // 相机跟随速度
        float const lookSpeed            = 4.0f; // 相机看向 player 的速度

        // 检查是否有手动相机控制
        bool const manualCameraControl = Input::isKey(KeyCode::ClickLeft) || Input::getThumbStickRightDistance() > 0.01f;

        if (manualCameraControl)
        {
            // 手动相机控制（右键按住时）
            float const mouseSensitivity      = 0.002f;
            float const thumbStickSensitivity = 5.0f;
            math::Vector2 const thumbStickDelta = Input::getThumbStickRight() * thumbStickSensitivity;
            math::Vector2 const delta = Input::getMouseDelta() + thumbStickDelta;

            float const yaw   = -delta.x * mouseSensitivity;
            float const pitch = -delta.y * mouseSensitivity;

            math::Quaternion const currentOrientation = camera->getOrientation();
            math::Quaternion const yawRotation        = math::Quaternion::fromAxisAngle(math::Vector3::Y(), yaw);
            math::Quaternion const pitchRotation      = math::Quaternion::fromAxisAngle(camera->getRight(), pitch);
            math::Quaternion const newOrientation     = yawRotation * pitchRotation * currentOrientation;
            camera->setOrientation(math::normalize(newOrientation));

            // 保持相机位置跟随，但不强制看向player
            math::Vector3 targetCameraPos  = playerPosition + cameraOffset;
            math::Vector3 currentCameraPos = camera->getPosition();
            math::Vector3 newCameraPos     = math::lerp(currentCameraPos, targetCameraPos, followSpeed * globalContext->deltaTime);
            camera->setPosition(newCameraPos);
        }
        else
        {
            // 自动相机追踪

            // 平滑移动相机位置
            math::Vector3 const newCameraPos = math::lerp(
                camera->getPosition(), // 当前相机位置
                playerPosition + cameraOffset, // 目标相机位置
                followSpeed * globalContext->deltaTime // lerp 增量
            );
            camera->setPosition(newCameraPos);

            // 计算相机应该看向的方向
            math::Vector3 const lookDirection = math::normalize(playerPosition - newCameraPos);
            math::Vector3 up                  = math::Vector3::Y();
            math::Vector3 const right         = math::normalize(math::cross(lookDirection, up));
            up                                = math::cross(right, lookDirection);

            // 平滑旋转相机
            math::Quaternion const newOrientation = math::sLerp(
                camera->getOrientation(), // 当前相机朝向
                math::Quaternion::fromMat3(math::Matrix3{right, up, -lookDirection}), // 目标相机朝向
                lookSpeed * globalContext->deltaTime // sLerp 增量
            );
            camera->setOrientation(newOrientation);
        }

        // 更新渲染器
        Renderer::setCameraPosition(camera->getPosition());
        Renderer::setCameraForward(camera->getForward());

        if (Input::isKeyDown(KeyCode::P))
        {
            globalContext->isWireFrameMode = !globalContext->isWireFrameMode;
        }
    }
    // clang-format on

    // clang-format off
    static void setupScene(
        ecs::Commands commands,
        ecs::ResourceArray<StandardMaterial> materials,
        ecs::Resource<AssetServer> assetServer,
        ecs::ResourceArray<Mesh> meshes
    )
    {
        root = commands.spawn(Children{});

        assetServer->load();

        // auto marble = materials->add(StandardMaterial{
        //     .albedoTexture           = assetServer->submitLoading("/Users/w6rsty/Downloads/assests/PBR-Textures/stringy-marble-bl/stringy_marble_albedo.png"),
        //     .roughnessTexture        = assetServer->submitLoading("/Users/w6rsty/Downloads/assests/PBR-Textures/stringy-marble-bl/stringy_marble_Roughness.png"),
        //     .ambientOcclusionTexture = assetServer->submitLoading("/Users/w6rsty/Downloads/assests/PBR-Textures/stringy-marble-bl/stringy_marble_ao.png"),
        // });

        // auto vegetation = materials->add(StandardMaterial{
        //     .albedoTexture           = assetServer->submitLoading("/Users/w6rsty/Downloads/assests/PBR-Textures/square-block-vegetation-bl/square-blocks-vegetation_albedo.png"),
        //     .normalTexture           = assetServer->submitLoading("/Users/w6rsty/Downloads/assests/PBR-Textures/square-block-vegetation-bl/square-blocks-vegetation_normal-ogl.png"),
        //     .roughnessTexture        = assetServer->submitLoading("/Users/w6rsty/Downloads/assests/PBR-Textures/square-block-vegetation-bl/square-blocks-vegetation_roughness.png"),
        //     .ambientOcclusionTexture = assetServer->submitLoading("/Users/w6rsty/Downloads/assests/PBR-Textures/square-block-vegetation-bl/square-blocks-vegetation_ao.png"),
        // });

        // auto gold = materials->add(StandardMaterial{
        //     .albedoTexture           = assetServer->submitLoading("/Users/w6rsty/Downloads/assests/PBR-Textures/hammered-gold-bl/hammered-gold_albedo.png"),
        //     .normalTexture           = assetServer->submitLoading("/Users/w6rsty/Downloads/assests/PBR-Textures/hammered-gold-bl/hammered-gold_normal-ogl.png"),
        //     .metallic                = 1.0f,
        //     .roughnessTexture        = assetServer->submitLoading("/Users/w6rsty/Downloads/assests/PBR-Textures/hammered-gold-bl/hammered-gold_roughness.png"),
        //     .ambientOcclusionTexture = assetServer->submitLoading("/Users/w6rsty/Downloads/assests/PBR-Textures/hammered-gold-bl/hammered-gold_ao.png"),
        // });
        
        // floor
        commands.spawn(
            LocalTransform{
                .position = math::Vector3{5.0f, 0.0f, 0.0f},
                .scale = math::Vector3{10.0f, 0.1f, 10.0f}
            },
            Mesh3D{meshes.add(Quad3D{})},
            MeshMaterial{materials->add(StandardMaterial{})}
        );
        commands.spawn(
            LocalTransform{
                .position = math::Vector3{-5.0f, 0.0f, 0.0f},
                .scale = math::Vector3{10.0f, 0.1f, 10.0f}
            },
            Mesh3D{meshes.add(Quad3D{})},
            MeshMaterial{materials->add(StandardMaterial{})}
        );


        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> posDist(-3.0f, 3.0f);
        std::uniform_real_distribution<float> heightDist(1.0f, 5.0f);

        std::vector<RHIVertexPosUvNrmTan> vertices;
        vertices.reserve(1000000);

        for (int i = 0; i < 1000000; ++i)
        {
            vertices.push_back(RHIVertexPosUvNrmTan{
                .position = math::Vector3{posDist(gen), heightDist(gen), posDist(gen)},
                .uv = math::Vector2{0.0f, 0.0f},
            });
        }

        player = commands.spawn(
            LocalTransform{
                .position = math::Vector3{0.0f, 10.0f, 0.0f},
            },
            Mesh3D{meshes.add(Capsule3D{
                .segments = 16,
            })},
            MeshMaterial{materials->add(StandardMaterial{})}
        );
    }
    // clang-format on

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

int main()
{
    Logger::initialize();
    ecs::Registry registry;
    ecs::Schedule schedule;

    schedule.addSystem<ecs::CoreStage::StartUp, &Engine::initialize>();
    schedule.addSystem<ecs::CoreStage::StartUp, &Renderer::initialize>();
    schedule.addSystem<ecs::CoreStage::StartUp, &World::initialize>();
    schedule.addSystem<ecs::CoreStage::StartUp, &World::setupScene>();
    schedule.addSystem<ecs::CoreStage::StartUp, buildMeshes>();
    schedule.addSystem<ecs::CoreStage::StartUp, buildMaterials>();
    schedule.addSystem<ecs::CoreStage::StartUp, &ImGuiRenderer::initialize>();

    schedule.addSystem<ecs::CoreStage::Update, &Engine::tick>();
    schedule.addSystem<ecs::CoreStage::Update, &World::inputControll>();
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
