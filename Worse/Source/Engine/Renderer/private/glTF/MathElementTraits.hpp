/**
 * @file MathElementTraits.hpp
 * @brief Specialize for fastgltf ElementTraits
 */

#pragma once
#include "base_type.hpp"
#include "math/vector.hpp"
#include "math/matrix.hpp"

#include "fastgltf/tools.hpp"

namespace fastgltf
{

    template <>
    struct ElementTraits<Worse::Vector2> : ElementTraitsBase<Worse::Vector2, AccessorType::Vec2, ::Worse::Float>
    {
    };

    template <>
    struct ElementTraits<Worse::Vector3> : ElementTraitsBase<Worse::Vector3, AccessorType::Vec3, ::Worse::Float>
    {
    };

    template <>
    struct ElementTraits<Worse::Vector4> : ElementTraitsBase<Worse::Vector4, AccessorType::Vec4, ::Worse::Float>
    {
    };

    template <>
    struct ElementTraits<Worse::Matrix2> : ElementTraitsBase<Worse::Matrix2, AccessorType::Mat2, ::Worse::Float>
    {
    };

    template <>
    struct ElementTraits<Worse::Matrix3> : ElementTraitsBase<Worse::Matrix3, AccessorType::Mat3, ::Worse::Float>
    {
    };

    template <>
    struct ElementTraits<Worse::Matrix4> : ElementTraitsBase<Worse::Matrix4, AccessorType::Mat4, ::Worse::Float>
    {
    };

} // namespace fastgltf
