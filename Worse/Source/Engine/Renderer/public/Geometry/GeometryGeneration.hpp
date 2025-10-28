/**
 * @file GeometryGeneration.hpp
 * @brief Geometry generation utilities for 3D shapes. Using CCW winding order.
 */

#pragma once
#include "Math/Math.hpp"
#include "RHITypes.hpp"

#include <vector>

namespace worse::geometry
{

    enum class GeometryType
    {
        Quad3D,
        Cube,
        Sphere,
        Cylinder,
        Capsule,
        Max
    };

    static void generateQuad3D(std::vector<RHIVertexPosUvNrmTan>& vertices,
                               std::vector<u32>& indices, f32 width = 1.0f,
                               f32 height = 1.0f)
    {
        using worse::math::Vector2;
        using worse::math::Vector3;
        using worse::math::Vector4;

        vertices.clear();
        indices.clear();
        vertices.reserve(4);
        indices.reserve(6);

        // clang-format off
        //                      +Y    +Z  
        //                       ^    ^  
        //              0        |   /      1
        //               \\      |  /      //
        //                 +-----|-+--------+
        //                /      |/        /
        //   -X - - - - -/ - - - * - - - -/- - - - > +X
        //              /       /|       /
        //             +-------+-|------+
        //           //       /  |    \\
        //           2       /   |      3
        //                  -Z   -Y
        // clang-format on

        vertices.emplace_back(Vector3(-0.5f * width, 0.0f, 0.5f * height), Vector2(0, 0), Vector3(0, 1, 0), Vector4(1, 0, 0, 1));  // 0 top-left
        vertices.emplace_back(Vector3(0.5f * width, 0.0f, 0.5f * height), Vector2(1, 0), Vector3(0, 1, 0), Vector4(1, 0, 0, 1));   // 1 top-right
        vertices.emplace_back(Vector3(-0.5f * width, 0.0f, -0.5f * height), Vector2(0, 1), Vector3(0, 1, 0), Vector4(1, 0, 0, 1)); // 2 bottom-left
        vertices.emplace_back(Vector3(0.5f * width, 0.0f, -0.5f * height), Vector2(1, 1), Vector3(0, 1, 0), Vector4(1, 0, 0, 1));  // 3 bottom-right

        indices.emplace_back(3);
        indices.emplace_back(2);
        indices.emplace_back(0);
        indices.emplace_back(3);
        indices.emplace_back(0);
        indices.emplace_back(1);
    }

    static void generateCube(std::vector<RHIVertexPosUvNrmTan>& vertices,
                             std::vector<u32>& indices, f32 width = 1.0f,
                             f32 height = 1.0f, f32 depth = 1.0f)
    {
        using worse::math::Vector2;
        using worse::math::Vector3;
        using worse::math::Vector4;

        vertices.clear();
        indices.clear();
        vertices.reserve(24);
        indices.reserve(36);

        // front
        vertices.emplace_back(Vector3(-0.5f, -0.5f, -0.5f), Vector2(0, 1), Vector3(0, 0, -1), Vector4(0, 1, 0, 1));
        vertices.emplace_back(Vector3(-0.5f, 0.5f, -0.5f), Vector2(0, 0), Vector3(0, 0, -1), Vector4(0, 1, 0, 1));
        vertices.emplace_back(Vector3(0.5f, -0.5f, -0.5f), Vector2(1, 1), Vector3(0, 0, -1), Vector4(0, 1, 0, 1));
        vertices.emplace_back(Vector3(0.5f, 0.5f, -0.5f), Vector2(1, 0), Vector3(0, 0, -1), Vector4(0, 1, 0, 1));

        // bottom
        vertices.emplace_back(Vector3(-0.5f, -0.5f, 0.5f), Vector2(0, 1), Vector3(0, -1, 0), Vector4(1, 0, 0, 1));
        vertices.emplace_back(Vector3(-0.5f, -0.5f, -0.5f), Vector2(0, 0), Vector3(0, -1, 0), Vector4(1, 0, 0, 1));
        vertices.emplace_back(Vector3(0.5f, -0.5f, 0.5f), Vector2(1, 1), Vector3(0, -1, 0), Vector4(1, 0, 0, 1));
        vertices.emplace_back(Vector3(0.5f, -0.5f, -0.5f), Vector2(1, 0), Vector3(0, -1, 0), Vector4(1, 0, 0, 1));

        // back
        vertices.emplace_back(Vector3(-0.5f, -0.5f, 0.5f), Vector2(1, 1), Vector3(0, 0, 1), Vector4(0, 1, 0, 1));
        vertices.emplace_back(Vector3(-0.5f, 0.5f, 0.5f), Vector2(1, 0), Vector3(0, 0, 1), Vector4(0, 1, 0, 1));
        vertices.emplace_back(Vector3(0.5f, -0.5f, 0.5f), Vector2(0, 1), Vector3(0, 0, 1), Vector4(0, 1, 0, 1));
        vertices.emplace_back(Vector3(0.5f, 0.5f, 0.5f), Vector2(0, 0), Vector3(0, 0, 1), Vector4(0, 1, 0, 1));

        // top
        vertices.emplace_back(Vector3(-0.5f, 0.5f, 0.5f), Vector2(0, 0), Vector3(0, 1, 0), Vector4(1, 0, 0, 1));
        vertices.emplace_back(Vector3(-0.5f, 0.5f, -0.5f), Vector2(0, 1), Vector3(0, 1, 0), Vector4(1, 0, 0, 1));
        vertices.emplace_back(Vector3(0.5f, 0.5f, 0.5f), Vector2(1, 0), Vector3(0, 1, 0), Vector4(1, 0, 0, 1));
        vertices.emplace_back(Vector3(0.5f, 0.5f, -0.5f), Vector2(1, 1), Vector3(0, 1, 0), Vector4(1, 0, 0, 1));

        // left
        vertices.emplace_back(Vector3(-0.5f, -0.5f, 0.5f), Vector2(0, 1), Vector3(-1, 0, 0), Vector4(0, 1, 0, 1));
        vertices.emplace_back(Vector3(-0.5f, 0.5f, 0.5f), Vector2(0, 0), Vector3(-1, 0, 0), Vector4(0, 1, 0, 1));
        vertices.emplace_back(Vector3(-0.5f, -0.5f, -0.5f), Vector2(1, 1), Vector3(-1, 0, 0), Vector4(0, 1, 0, 1));
        vertices.emplace_back(Vector3(-0.5f, 0.5f, -0.5f), Vector2(1, 0), Vector3(-1, 0, 0), Vector4(0, 1, 0, 1));

        // right
        vertices.emplace_back(Vector3(0.5f, -0.5f, 0.5f), Vector2(1, 1), Vector3(1, 0, 0), Vector4(0, 1, 0, 1));
        vertices.emplace_back(Vector3(0.5f, 0.5f, 0.5f), Vector2(1, 0), Vector3(1, 0, 0), Vector4(0, 1, 0, 1));
        vertices.emplace_back(Vector3(0.5f, -0.5f, -0.5f), Vector2(0, 1), Vector3(1, 0, 0), Vector4(0, 1, 0, 1));
        vertices.emplace_back(Vector3(0.5f, 0.5f, -0.5f), Vector2(0, 0), Vector3(1, 0, 0), Vector4(0, 1, 0, 1));

        // front
        indices.emplace_back(0);
        indices.emplace_back(1);
        indices.emplace_back(2);
        indices.emplace_back(2);
        indices.emplace_back(1);
        indices.emplace_back(3);

        // bottom
        indices.emplace_back(4);
        indices.emplace_back(5);
        indices.emplace_back(6);
        indices.emplace_back(6);
        indices.emplace_back(5);
        indices.emplace_back(7);

        // back
        indices.emplace_back(10);
        indices.emplace_back(9);
        indices.emplace_back(8);
        indices.emplace_back(11);
        indices.emplace_back(9);
        indices.emplace_back(10);

        // top
        indices.emplace_back(14);
        indices.emplace_back(13);
        indices.emplace_back(12);
        indices.emplace_back(15);
        indices.emplace_back(13);
        indices.emplace_back(14);

        // left
        indices.emplace_back(16);
        indices.emplace_back(17);
        indices.emplace_back(18);
        indices.emplace_back(18);
        indices.emplace_back(17);
        indices.emplace_back(19);

        // right
        indices.emplace_back(22);
        indices.emplace_back(21);
        indices.emplace_back(20);
        indices.emplace_back(23);
        indices.emplace_back(21);
        indices.emplace_back(22);
    }

    static void generateSphere(std::vector<RHIVertexPosUvNrmTan>& vertices,
                               std::vector<u32>& indices, f32 radius = 0.5f,
                               u32 segments = 32, u32 rings = 16)
    {
        using worse::math::Vector2;
        using worse::math::Vector3;
        using worse::math::Vector4;

        vertices.clear();
        indices.clear();

        vertices.reserve((segments + 1) * (rings - 1) + 2);
        indices.reserve(segments * rings * 6);

        // --- Vertices ---

        // Top pole
        vertices.emplace_back(Vector3(0.0f, radius, 0.0f),
                              Vector2(0.5f, 0.0f),
                              Vector3(0.0f, 1.0f, 0.0f),
                              Vector4(1.0f, 0.0f, 0.0f, 1.0f));
        const u32 topPoleIndex = 0;

        // Rings (excluding poles)
        for (u32 r = 1; r < rings; ++r)
        {
            f32 phi    = math::PI * static_cast<f32>(r) / static_cast<f32>(rings);
            f32 sinPhi = std::sin(phi);
            f32 cosPhi = std::cos(phi);

            for (u32 s = 0; s <= segments; ++s)
            {
                f32 theta = 2.0f * math::PI * static_cast<f32>(s) /
                            static_cast<f32>(segments);
                f32 sinTheta = std::sin(theta);
                f32 cosTheta = std::cos(theta);

                Vector3 position(radius * sinPhi * cosTheta,
                                 radius * cosPhi,
                                 radius * sinPhi * sinTheta);
                Vector2 uv(static_cast<f32>(s) / static_cast<f32>(segments),
                           static_cast<f32>(r) / static_cast<f32>(rings));
                Vector3 normal = normalize(position);

                // CORRECTED: Tangent handedness should be consistent.
                // The bitangent can be calculated in the shader as:
                // bitangent = cross(normal, tangent.xyz) * tangent.w
                Vector4 tangent =
                    Vector4(normalize(Vector3(-sinTheta, 0.0f, cosTheta)), 1.0f);

                vertices.emplace_back(position, uv, normal, tangent);
            }
        }

        // Bottom pole
        vertices.emplace_back(Vector3(0.0f, -radius, 0.0f),
                              Vector2(0.5f, 1.0f),
                              Vector3(0.0f, -1.0f, 0.0f),
                              Vector4(1.0f, 0.0f, 0.0f, 1.0f));
        const u32 bottomPoleIndex = static_cast<u32>(vertices.size() - 1);

        // --- Indices ---

        // Top fan (CCW when viewed from outside)
        for (u32 s = 0; s < segments; ++s)
        {
            // vertex index for the first ring starts at 1 (after the top pole)
            u32 first  = 1 + s;
            u32 second = 1 + s + 1;
            // CORRECTED: Winding order was (pole, second, first), which is CW.
            // Swapped to (pole, first, second) for CCW.
            indices.emplace_back(topPoleIndex);
            indices.emplace_back(first);
            indices.emplace_back(second);
        }

        // Middle quads (CCW when viewed from outside)
        for (u32 r = 0; r < rings - 2; ++r)
        {
            for (u32 s = 0; s < segments; ++s)
            {
                // vertex index for the first ring starts at 1
                u32 first_row_start  = 1 + r * (segments + 1);
                u32 second_row_start = 1 + (r + 1) * (segments + 1);

                u32 i0 = first_row_start + s;
                u32 i1 = first_row_start + s + 1;
                u32 i2 = second_row_start + s;
                u32 i3 = second_row_start + s + 1;

                // CORRECTED: Winding order was CW. Swapped to CCW.
                // Original Triangle 1: i0, i2, i1 (CW)
                // Original Triangle 2: i1, i2, i3 (CW)
                indices.emplace_back(i0);
                indices.emplace_back(i1);
                indices.emplace_back(i2);

                indices.emplace_back(i2);
                indices.emplace_back(i1);
                indices.emplace_back(i3);
            }
        }

        // Bottom fan (CCW when viewed from outside)
        // vertex index for the start of the last ring
        u32 lastRingStartIndex = 1 + (rings - 2) * (segments + 1);
        for (u32 s = 0; s < segments; ++s)
        {
            u32 first  = lastRingStartIndex + s;
            u32 second = lastRingStartIndex + s + 1;
            // CORRECTED: Winding order was (pole, first, second), which is CW
            // when viewed from outside. Swapped to (pole, second, first) for CCW.
            indices.emplace_back(bottomPoleIndex);
            indices.emplace_back(second);
            indices.emplace_back(first);
        }
    }

    static void generateCylinder(std::vector<RHIVertexPosUvNrmTan>& vertices,
                                 std::vector<u32>& indices, f32 radius = 0.5f,
                                 f32 height = 1.0f, u32 segments = 32,
                                 u32 heightSegments = 1, bool topCap = true,
                                 bool bottomCap = true)
    {
        using worse::math::Vector2;
        using worse::math::Vector3;
        using worse::math::Vector4;

        vertices.clear();
        indices.clear();

        // 计算顶点数量：侧面 + 顶面 + 底面
        u32 sideVertices = (segments + 1) * (heightSegments + 1);
        u32 capVertices  = topCap ? (segments + 2) : 0; // 中心点 + 边缘点
        capVertices += bottomCap ? (segments + 2) : 0;

        vertices.reserve(sideVertices + capVertices);

        // 计算索引数量
        u32 sideIndices = segments * heightSegments * 6;
        u32 capIndices =
            (topCap ? segments * 3 : 0) + (bottomCap ? segments * 3 : 0);
        indices.reserve(sideIndices + capIndices);

        f32 halfHeight = height * 0.5f;

        // 生成侧面顶点
        for (u32 h = 0; h <= heightSegments; ++h)
        {
            f32 y = -halfHeight + (height * static_cast<f32>(h) /
                                   static_cast<f32>(heightSegments));
            f32 v = static_cast<f32>(h) / static_cast<f32>(heightSegments);

            for (u32 s = 0; s <= segments; ++s)
            {
                f32 theta = 2.0f * math::PI * static_cast<f32>(s) /
                            static_cast<f32>(segments);
                f32 x = radius * std::cos(theta);
                f32 z = radius * std::sin(theta);
                f32 u = static_cast<f32>(s) / static_cast<f32>(segments);

                Vector3 position(x, y, z);
                Vector2 uv(u, v);
                Vector3 normal   = normalize(Vector3(x, 0.0f, z)); // 径向法线
                float handedness = (s % 2 == 0) ? 1.0f : -1.0f;
                Vector4 tangent  = Vector4(normalize(Vector3(-std::sin(theta), 0.0f, std::cos(theta))), handedness);

                vertices.emplace_back(position, uv, normal, tangent);
            }
        }

        // 生成侧面索引 (CCW when viewed from outside)
        for (u32 h = 0; h < heightSegments; ++h)
        {
            for (u32 s = 0; s < segments; ++s)
            {
                u32 first  = h * (segments + 1) + s;
                u32 second = first + segments + 1;

                indices.emplace_back(first);
                indices.emplace_back(second);
                indices.emplace_back(first + 1);

                indices.emplace_back(first + 1);
                indices.emplace_back(second);
                indices.emplace_back(second + 1);
            }
        }

        u32 currentVertexIndex = sideVertices;

        // 生成顶盖
        if (topCap)
        {
            // 顶盖中心点
            Vector3 topCenter(0.0f, halfHeight, 0.0f);
            Vector2 topCenterUv(0.5f, 0.5f);
            Vector3 topNormal(0.0f, 1.0f, 0.0f);
            Vector4 topTangent(1.0f, 0.0f, 0.0f, 1.0f);

            u32 topCenterIndex = currentVertexIndex;
            vertices.emplace_back(topCenter,
                                  topCenterUv,
                                  topNormal,
                                  topTangent);
            currentVertexIndex++;

            // 顶盖边缘点
            for (u32 s = 0; s <= segments; ++s)
            {
                f32 theta = 2.0f * math::PI * static_cast<f32>(s) /
                            static_cast<f32>(segments);
                f32 x = radius * std::cos(theta);
                f32 z = radius * std::sin(theta);

                Vector3 position(x, halfHeight, z);
                Vector2 uv(0.5f + 0.5f * std::cos(theta),
                           0.5f + 0.5f * std::sin(theta));
                Vector3 normal(0.0f, 1.0f, 0.0f);
                float handedness = (s % 2 == 0) ? 1.0f : -1.0f;
                Vector4 tangent(1.0f, 0.0f, 0.0f, handedness);

                vertices.emplace_back(position, uv, normal, tangent);
            }

            // 顶盖索引 (CCW when viewed from above)
            for (u32 s = 0; s < segments; ++s)
            {
                indices.emplace_back(topCenterIndex);
                indices.emplace_back(currentVertexIndex + s);
                indices.emplace_back(currentVertexIndex + s + 1);
            }
            currentVertexIndex += segments + 1;
        }

        // 生成底盖
        if (bottomCap)
        {
            // 底盖中心点
            Vector3 bottomCenter(0.0f, -halfHeight, 0.0f);
            Vector2 bottomCenterUv(0.5f, 0.5f);
            Vector3 bottomNormal(0.0f, -1.0f, 0.0f);
            Vector4 bottomTangent(1.0f, 0.0f, 0.0f, 1.0f);

            u32 bottomCenterIndex = currentVertexIndex;
            vertices.emplace_back(bottomCenter,
                                  bottomCenterUv,
                                  bottomNormal,
                                  bottomTangent);
            currentVertexIndex++;

            // 底盖边缘点
            for (u32 s = 0; s <= segments; ++s)
            {
                f32 theta = 2.0f * math::PI * static_cast<f32>(s) /
                            static_cast<f32>(segments);
                f32 x = radius * std::cos(theta);
                f32 z = radius * std::sin(theta);

                Vector3 position(x, -halfHeight, z);
                Vector2 uv(0.5f + 0.5f * std::cos(theta),
                           0.5f - 0.5f * std::sin(theta));
                Vector3 normal(0.0f, -1.0f, 0.0f);
                Vector4 tangent(1.0f, 0.0f, 0.0f, 1.0f);

                vertices.emplace_back(position, uv, normal, tangent);
            }

            // 底盖索引 (CCW when viewed from below)
            for (u32 s = 0; s < segments; ++s)
            {
                indices.emplace_back(bottomCenterIndex);
                indices.emplace_back(currentVertexIndex + s + 1);
                indices.emplace_back(currentVertexIndex + s);
            }
        }
    }

    static void generateCapsule(std::vector<RHIVertexPosUvNrmTan>& vertices,
                                std::vector<u32>& indices, f32 radius = 0.5f,
                                f32 height = 2.0f, u32 segments = 32,
                                u32 totalRings = 16)
    {
        using worse::math::Vector2;
        using worse::math::Vector3;
        using worse::math::Vector4;

        vertices.clear();
        indices.clear();

        const f32 cylinderHeight     = std::max(0.0f, height - 2.0f * radius);
        const f32 halfCylinderHeight = cylinderHeight * 0.5f;
        const u32 hemisphereRings    = totalRings / 2;

        // --- Arc-length calculation for proportional V coordinate ---
        const f32 hemisphereArcLength = math::PI * 0.5f * radius;
        const f32 totalArcLength      = 2.0f * hemisphereArcLength + cylinderHeight;

        // --- Vertices ---
        // A single loop to generate all vertices seamlessly
        // We add 3 rings: top pole, junction between top cap and cylinder,
        // junction between cylinder and bottom cap, bottom pole
        u32 rings = hemisphereRings * 2 + 1; // total rings for the whole shape
        if (cylinderHeight > 0)
            rings += 1; // an extra ring for the cylinder middle if it exists

        for (u32 r = 0; r <= rings; ++r)
        {
            f32 v_ratio        = static_cast<f32>(r) / static_cast<f32>(rings);
            f32 currentArcDist = v_ratio * totalArcLength;

            f32 phi, y, effectiveRadius;
            Vector3 normal_y_dir;

            // Determine which part of the capsule the ring is on
            if (currentArcDist < hemisphereArcLength) // Top hemisphere
            {
                phi             = currentArcDist / radius;
                effectiveRadius = radius * std::sin(phi);
                y               = radius * std::cos(phi) + halfCylinderHeight;
                normal_y_dir    = Vector3(0, std::cos(phi), 0);
            }
            else if (currentArcDist >
                     hemisphereArcLength + cylinderHeight) // Bottom hemisphere
            {
                f32 arcDist_from_bottom = totalArcLength - currentArcDist;
                phi                     = arcDist_from_bottom / radius;
                effectiveRadius         = radius * std::sin(phi);
                y                       = -(radius * std::cos(phi) + halfCylinderHeight);
                normal_y_dir            = Vector3(0, -std::cos(phi), 0);
            }
            else // Cylinder body
            {
                effectiveRadius = radius;
                y               = halfCylinderHeight - (currentArcDist - hemisphereArcLength);
                normal_y_dir    = Vector3(0, 0, 0);
            }

            for (u32 s = 0; s <= segments; ++s)
            {
                f32 u        = static_cast<f32>(s) / static_cast<f32>(segments);
                f32 theta    = 2.0f * math::PI * u;
                f32 cosTheta = std::cos(theta);
                f32 sinTheta = std::sin(theta);

                f32 x = effectiveRadius * cosTheta;
                f32 z = effectiveRadius * sinTheta;

                Vector3 position(x, y, z);
                Vector2 uv(u, v_ratio);
                Vector3 normal   = normalize(Vector3(x, 0.0f, z) + normal_y_dir * radius);
                float handedness = (s % 2 == 0) ? 1.0f : -1.0f;
                Vector4 tangent  = Vector4(normalize(Vector3(-sinTheta, 0.0f, cosTheta)), handedness);

                vertices.emplace_back(position, uv, normal, tangent);
            }
        }

        // --- Indices --- (CCW when viewed from outside)
        for (u32 r = 0; r < rings; ++r)
        {
            for (u32 s = 0; s < segments; ++s)
            {
                u32 i0 = r * (segments + 1) + s;
                u32 i1 = i0 + 1;
                u32 i2 = (r + 1) * (segments + 1) + s;
                u32 i3 = i2 + 1;

                indices.emplace_back(i0);
                indices.emplace_back(i2);
                indices.emplace_back(i1);

                indices.emplace_back(i1);
                indices.emplace_back(i2);
                indices.emplace_back(i3);
            }
        }
    }

} // namespace worse::geometry