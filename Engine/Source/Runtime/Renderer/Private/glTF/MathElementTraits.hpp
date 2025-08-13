/**
 * @file MathElementTraits.hpp
 * @brief 提供数学库为 fastgltf 提供的 ElementTraits 特化
 */

#pragma once
#include "Math/Vector.hpp"
#include "Math/Matrix.hpp"

#include "fastgltf/tools.hpp"

namespace fastgltf
{

    template <>
    struct ElementTraits<worse::math::Vector2> : ElementTraitsBase<worse::math::Vector2, AccessorType::Vec2, f32>
    {
    };

    template <>
    struct ElementTraits<worse::math::Vector3> : ElementTraitsBase<worse::math::Vector3, AccessorType::Vec3, f32>
    {
    };

    template <>
    struct ElementTraits<worse::math::Vector4> : ElementTraitsBase<worse::math::Vector4, AccessorType::Vec4, f32>
    {
    };

    template <>
    struct ElementTraits<worse::math::Vector2i> : ElementTraitsBase<worse::math::Vector2i, AccessorType::Vec2, i32>
    {
    };

    template <>
    struct ElementTraits<worse::math::Vector3i> : ElementTraitsBase<worse::math::Vector3i, AccessorType::Vec3, i32>
    {
    };

    template <>
    struct ElementTraits<worse::math::Vector4i> : ElementTraitsBase<worse::math::Vector4i, AccessorType::Vec4, i32>
    {
    };

    template <>
    struct ElementTraits<worse::math::Matrix2> : ElementTraitsBase<worse::math::Matrix2, AccessorType::Mat2, f32>
    {
    };

    template <>
    struct ElementTraits<worse::math::Matrix3> : ElementTraitsBase<worse::math::Matrix3, AccessorType::Mat3, f32>
    {
    };

    template <>
    struct ElementTraits<worse::math::Matrix4> : ElementTraitsBase<worse::math::Matrix4, AccessorType::Mat4, f32>
    {
    };

} // namespace fastgltf
