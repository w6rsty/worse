#include "imgui.h"

#include "Platform.hpp"
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
#include "glTF/glTF.hpp"

#include "ECS/Commands.hpp"
#include "ECS/QueryView.hpp"
#include "ECS/Resource.hpp"
#include "ECS/Registry.hpp"
#include "ECS/Schedule.hpp"

struct A
{
    int a;
    char c;
};

using namespace worse;

class World
{
public:
    enum class State
    {
        Undefined,
        Begin,
        Main,
        End,
    };

    inline static ecs::Entity player         = ecs::Entity::null();
    inline static float cameraMoveSpeed      = 5.0f;
    inline static float cameraLookSpeed      = 5.0f;
    inline static math::Vector3 cameraOffset = math::Vector3{0.0f, 0.2f, 2.5f}; // 相机相对于 player 的偏移

    inline static std::unique_ptr<Mesh> customMesh = nullptr;

    static void initialize(
        ecs::Commands commands,
        ecs::ResourceArray<StandardMaterial> materials,
        ecs::Resource<glTFManager> gltfManager)
    {
        // clang-format off
        Camera& camera = commands.emplaceResource<Camera>()
            .setPosition(math::Vector3{0.0f, 10.0f, 20.0f})
            .setPerspectiveParams(math::toRadians(65.0f), static_cast<float>(Window::getWidth()) / static_cast<float>(Window::getHeight()), 0.1f, 100.0f);

        // 订阅窗口事件更新 AspectRatio
        EventBus::subscribe(EventType::WindowResized,
        [&camera](Event const& payload)
        {
            camera.setAspectRatio(
                static_cast<float>(Window::getWidth()) / static_cast<float>(Window::getHeight())
            );
        });

        PageRouter<State>& router = ImGuiRenderer::registerStates<State>(commands, State::Begin);
        router.registerPage(State::Begin, [&router](ecs::Commands commands, ecs::Resource<GlobalContext> globalContext)
        {
            ImGui::Begin("图形已死");

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
            ImGui::Text("W/A/S/D");
            ImGui::PopStyleColor();
            ImGui::SameLine();
            ImGui::Text("Move around");

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.8f, 0.2f, 1.0f));
            ImGui::Text("Q/E");
            ImGui::PopStyleColor();
            ImGui::SameLine();
            ImGui::Text("Move down/up");

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.2f, 0.8f, 1.0f));
            ImGui::Text("P");
            ImGui::PopStyleColor();
            ImGui::SameLine();
            ImGui::Text("Toggle wireframe mode");

            ImGui::Text("LEFT CLICK and MOUSE MOVE to look around");

            if (ImGui::Button("Next page"))
            {
                router.transfer(State::Main);
            }

            ImGui::End();
        });

        router.registerPage(State::Main, [&router](ecs::Commands commands, ecs::Resource<GlobalContext> globalContext)
        {
            ImGui::Begin("Main Page");

            ImGui::Text("This is the main page of the example application.");
            ImGui::Text("You can add more functionality here.");

            if (ImGui::Button("Back"))
            {
                router.transfer(State::Begin);
            }

            ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
            if (ImGui::Button("Exit"))
            {
                Window::close();
            }
            ImGui::PopStyleColor();

            ImGui::End();
        });

        // clang-format on

        gltfManager->load(std::string(EngineDirectory) + "/Binary/Models/DamagedHelmet/glTF-Binary/DamagedHelmet.glb", "helmet");
    }

    static void inputControll(
        ecs::Commands commands,
        ecs::Resource<Camera> camera,
        ecs::Resource<GlobalContext> globalContext)
    {
        if (Input::isKeyDown(KeyCode::Escape))
        {
            Window::close();
        }

        // 获取player的位置和变换
        LocalTransform& playerTransform = commands.getComponent<LocalTransform>(player);
        math::Vector3 playerPosition    = playerTransform.position;

        float const moveSpeed = cameraMoveSpeed * globalContext->deltaTime;

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
        if (Input::isKey(KeyCode::E))
        {
            playerPosition.y += moveSpeed; // 向上
        }
        if (Input::isKey(KeyCode::Q))
        {
            playerPosition.y -= moveSpeed; // 向下
        }

        // 更新 player 位置
        playerTransform.position = playerPosition;

        // 检查是否有手动相机控制
        bool const manualCameraControl = Input::isKey(KeyCode::ClickLeft) || Input::getThumbStickRightDistance() > 0.01f;

        if (manualCameraControl)
        {
            // 手动相机控制（右键按住时）
            float const mouseSensitivity        = 0.002f;
            float const thumbStickSensitivity   = 5.0f;
            math::Vector2 const thumbStickDelta = Input::getThumbStickRight() * thumbStickSensitivity;
            math::Vector2 const delta           = Input::getMouseDelta() + thumbStickDelta;

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
            float const lerpFactor         = std::max(0.01f, cameraLookSpeed * globalContext->deltaTime);
            math::Vector3 newCameraPos     = math::lerp(currentCameraPos, targetCameraPos, lerpFactor);
            camera->setPosition(newCameraPos);
        }
        else
        {
            // 自动相机追踪

            // 平滑移动相机位置
            float const positionLerpFactor   = std::max(0.01f, cameraLookSpeed * globalContext->deltaTime);
            math::Vector3 const newCameraPos = math::lerp(
                camera->getPosition(),         // 当前相机位置
                playerPosition + cameraOffset, // 目标相机位置
                positionLerpFactor             // lerp 增量（带最小值clamp）
            );
            camera->setPosition(newCameraPos);

            // 计算相机应该看向的方向
            math::Vector3 const lookDirection = math::normalize(playerPosition - newCameraPos);
            math::Vector3 up                  = math::Vector3::Y();
            math::Vector3 const right         = math::normalize(math::cross(lookDirection, up));
            up                                = math::cross(right, lookDirection);

            // 平滑旋转相机
            float const rotationLerpFactor        = std::max(0.01f, cameraLookSpeed * globalContext->deltaTime);
            math::Quaternion const newOrientation = math::sLerp(
                camera->getOrientation(),                                             // 当前相机朝向
                math::Quaternion::fromMat3(math::Matrix3{right, up, -lookDirection}), // 目标相机朝向
                rotationLerpFactor                                                    // sLerp 增量（带最小值clamp）
            );
            camera->setOrientation(newOrientation);
        }

        if (Input::isKeyDown(KeyCode::P))
        {
            globalContext->isWireFrameMode = !globalContext->isWireFrameMode;
        }
    }

    static void setupScene(
        ecs::Commands commands,
        ecs::ResourceArray<StandardMaterial> materials,
        ecs::Resource<AssetServer> assetServer)
    {
        // ground
        commands.spawn(
            LocalTransform{
                .position = math::Vector3{0, -1, 0},
                .scale    = math::Vector3(10, 1, 10),
            },
            Mesh3D{Renderer::getStandardMesh(geometry::GeometryType::Quad3D)},
            MeshMaterial{materials->add(StandardMaterial{
                .baseColor = math::Vector4(0.2f, 0.2f, 0.2f, 1.0f),
            })});

        // cubes
        for (int i = 0; i < 10; ++i)
        {
            commands.spawn(
                LocalTransform{
                    .position = math::Vector3{
                        static_cast<float>(rand() % 10 - 5),
                        static_cast<float>(rand() % 5),
                        static_cast<float>(rand() % 10 - 5)},
                    .rotation = math::Quaternion::fromEuler(math::Vector3(static_cast<float>(rand() % 360), static_cast<float>(rand() % 360), static_cast<float>(rand() % 360))),
                    .scale    = math::Vector3::splat(0.8f),
                },
                Mesh3D{Renderer::getStandardMesh(geometry::GeometryType::Cube)},
                MeshMaterial{materials->add(StandardMaterial{
                    .baseColor = math::Vector4(0.8f, 0.2f, 0.2f, 1.0f),
                })});
        }

        player = commands.spawn(
            LocalTransform{
                .scale = math::Vector3::splat(0.5f),
            },
            Mesh3D{Renderer::getStandardMesh(geometry::GeometryType::Capsule), RHIPrimitiveTopology::PointList},
            MeshMaterial{materials->add(StandardMaterial{
                .baseColor = math::Vector4(5.0f, 1.0f, 2.0f, 1.0f),
            })});
    }

    static void drawglTFModel(ecs::Resource<glTFManager> gltfManager, ecs::Resource<DrawcallStorage> drawcallStorage)
    {
        math::Matrix4 xform1 = math::makeSRT(
            math::Vector3::ONE(),
            math::Quaternion::fromAxisAngle(math::Vector3::Y(), math::toRadians(45.0f)),
            math::Vector3::ZERO());
        drawModel("helmet", xform1, *gltfManager.get(), drawcallStorage->ctx);
    }

    static void update(
        ecs::Commands commands,
        ecs::QueryView<LocalTransform> view,
        ecs::Resource<GlobalContext> globalContext,
        ecs::Resource<Camera> camera)
    {
        static profiling::Stopwatch frameTimer;
        globalContext->deltaTime = frameTimer.elapsedSec();
        globalContext->time += globalContext->deltaTime;
        frameTimer.reset();
    }
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
    schedule.addSystem<ecs::CoreStage::StartUp, buildMaterials>();
    schedule.addSystem<ecs::CoreStage::StartUp, &ImGuiRenderer::initialize>();

    schedule.addSystem<ecs::CoreStage::Update, &Engine::tick>();
    schedule.addSystem<ecs::CoreStage::Update, &World::inputControll>();
    schedule.addSystem<ecs::CoreStage::Update, &World::update>();
    schedule.addSystem<ecs::CoreStage::Update, &World::drawglTFModel>();
    schedule.addSystem<ecs::CoreStage::Update, &ImGuiRenderer::tick>();
    schedule.addSystem<ecs::CoreStage::Update, buildDrawcalls>();
    schedule.addSystem<ecs::CoreStage::Update, &Renderer::tick>();

    schedule.addSystem<ecs::CoreStage::CleanUp, &ImGuiRenderer::shutdown>();
    schedule.addSystem<ecs::CoreStage::CleanUp, &Renderer::shutdown>();
    schedule.addSystem<ecs::CoreStage::CleanUp, &Engine::shutdown>();

    profiling::Stopwatch mainTimer;
    schedule.initialize(registry);
    WS_LOG_INFO("Main", "ECS schedule initialized in {} ms", mainTimer.elapsedMs());

    while (!Window::shouldClose())
    {
        schedule.run(registry);
    }
    schedule.shutdown(registry);

    Logger::shutdown();
}
