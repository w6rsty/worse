#include "Input/Input.hpp"
#include "Renderer.hpp"
#include "World.hpp"

// clang-format off
void World::updateInput(
    ecs::Resource<Camera> camera,
    ecs::Resource<GlobalContext> globalContext
)
{
    if (Input::isKeyDown(KeyCode::Escape))
    {
        Window::close();
    }

    float const moveSpeed = cameraMoveSpeed * globalContext->deltaTime;
    // WASD控制相机移动
    math::Vector3 moveDirection = math::Vector3::ZERO();

    if (Input::isKey(KeyCode::W))
    {
        moveDirection += camera->getForward();
    }
    if (Input::isKey(KeyCode::S))
    {
        moveDirection -= camera->getForward();
    }
    if (Input::isKey(KeyCode::A))
    {
         moveDirection -= camera->getRight();
    }
    if (Input::isKey(KeyCode::D))
    {
         moveDirection += camera->getRight();
    }

    if (math::length(moveDirection) > 0.0f)
    {
        moveDirection = math::normalize(moveDirection) * moveSpeed;
    }

    // 垂直移动
    if (Input::isKey(KeyCode::Q))
    {
        moveDirection.y += moveSpeed;
    }
    if (Input::isKey(KeyCode::E))
    {
        moveDirection.y -= moveSpeed;
    }

    camera->setPosition(camera->getPosition() + moveDirection);

    if (Input::isKey(KeyCode::ClickRight))
    {
        float const mouseSensitivity = 0.002f;
        math::Vector2 const delta = Input::getMouseDelta();

        float const yaw   = -delta.x * mouseSensitivity;
        float const pitch = -delta.y * mouseSensitivity;

        math::Quaternion const currentOrientation = camera->getOrientation();
        math::Quaternion const yawRotation = math::Quaternion::fromAxisAngle(math::Vector3::Y(), yaw);
        math::Quaternion const pitchRotation = math::Quaternion::fromAxisAngle(camera->getRight(), pitch);
        math::Quaternion const newOrientation = yawRotation * pitchRotation * currentOrientation;
        camera->setOrientation(math::normalize(newOrientation));
        cameraOrientation = camera->getOrientation(); // 这会打断sLerp
    }
    else
    {
        // 平滑旋转相机
        math::Quaternion const realOrientation = math::sLerp(
            camera->getOrientation(),
            cameraOrientation,
            cameraFollowSpeed * globalContext->deltaTime // sLerp 增量
        );
        camera->setOrientation(realOrientation);
    }

    // 更新渲染器
    Renderer::setCameraPosition(camera->getPosition());
    Renderer::setCameraForward(camera->getForward());
}
// clang-format on