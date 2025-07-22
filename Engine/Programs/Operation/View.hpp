#pragma once
#include "Camera.hpp"
#include "../Application/World.hpp"

void setPerspectiveView(worse::Camera* camera);
void setOrthographicView(worse::Camera* camera, float scale);
void setTopView(worse::Camera* camera, CameraData* cameraData, float scale);
void setSideView(worse::Camera* camera, CameraData* cameraData, float scale);
// 根据焦点和缩放比例调整相机视图
void fitView(worse::Camera* camera, CameraData* cameraData, worse::math::Vector3 const& focus, float scale);