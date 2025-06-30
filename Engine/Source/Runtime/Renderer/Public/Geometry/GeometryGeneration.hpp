#pragma once
#include "Math/Math.hpp"
#include "RHITypes.hpp"

#include <vector>
#include <cstdint>

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
                               std::vector<std::uint32_t>& indices,
                               float width = 1.0f, float height = 1.0f)
    {
        using worse::math::Vector2;
        using worse::math::Vector3;

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

        vertices.emplace_back(Vector3(-0.5f * width,  0.0f,  0.5f * height), Vector2(0, 0), Vector3(0, 1, 0), Vector3(1, 0, 0)); // 0 top-left
        vertices.emplace_back(Vector3( 0.5f * width,  0.0f,  0.5f * height), Vector2(1, 0), Vector3(0, 1, 0), Vector3(1, 0, 0)); // 1 top-right
        vertices.emplace_back(Vector3(-0.5f * width,  0.0f, -0.5f * height), Vector2(0, 1), Vector3(0, 1, 0), Vector3(1, 0, 0)); // 2 bottom-left
        vertices.emplace_back(Vector3( 0.5f * width,  0.0f, -0.5f * height), Vector2(1, 1), Vector3(0, 1, 0), Vector3(1, 0, 0)); // 3 bottom-right

        indices.emplace_back(3);
        indices.emplace_back(2);
        indices.emplace_back(0);
        indices.emplace_back(3);
        indices.emplace_back(0);
        indices.emplace_back(1);
        // clang-format on
    }

    static void generateCube(std::vector<RHIVertexPosUvNrmTan>& vertices,
                             std::vector<std::uint32_t>& indices,
                             float width = 1.0f, float height = 1.0f,
                             float depth = 1.0f)
    {
        using worse::math::Vector2;
        using worse::math::Vector3;

        vertices.clear();
        indices.clear();
        vertices.reserve(24);
        indices.reserve(36);

        // clang-format off
    
        //                           +Y       +Z
        //                            ^       ^
        //             (9/12/17)      |      /    (11/14/21)
        //                     \\     |     /    //
        //                       +----|----/----+
        //                      /|    |   /    /|
        //                     / | (3/15/23)  / |
        //                    /  |    | / \\ /  |
        //      (1/13/19) == +---+----|/----+   |
        //     -X - - - - - -|- -|- - * - - |- -|- - - - - - > +X
        //                   |   +---/|-----+---+ == (6/10/20)
        //                   |  / \\/ |     |  /
        //                   | /   (4/8/16) | /
        //                   |/   /   |     |/
        //                   +---/----|-----+
        //                 //   /     |     \\
        //          (0/5/18)   /      |      (2/7/22)
        //                   -Z       -Y     

        // front
        vertices.emplace_back(Vector3(-0.5f * width, -0.5f * height, -0.5f * depth), Vector2(0, 1), Vector3(0, 0, -1), Vector3(0, 1, 0)); // 0
        vertices.emplace_back(Vector3(-0.5f * width,  0.5f * height, -0.5f * depth), Vector2(0, 0), Vector3(0, 0, -1), Vector3(0, 1, 0)); // 1
        vertices.emplace_back(Vector3( 0.5f * width, -0.5f * height, -0.5f * depth), Vector2(1, 1), Vector3(0, 0, -1), Vector3(0, 1, 0)); // 2
        vertices.emplace_back(Vector3( 0.5f * width,  0.5f * height, -0.5f * depth), Vector2(1, 0), Vector3(0, 0, -1), Vector3(0, 1, 0)); // 3

        // bottom
        vertices.emplace_back(Vector3(-0.5f * width, -0.5f * height,  0.5f * depth), Vector2(0, 1), Vector3(0, -1, 0), Vector3(1, 0, 0)); // 4
        vertices.emplace_back(Vector3(-0.5f * width, -0.5f * height, -0.5f * depth), Vector2(0, 0), Vector3(0, -1, 0), Vector3(1, 0, 0)); // 5
        vertices.emplace_back(Vector3( 0.5f * width, -0.5f * height,  0.5f * depth), Vector2(1, 1), Vector3(0, -1, 0), Vector3(1, 0, 0)); // 6
        vertices.emplace_back(Vector3( 0.5f * width, -0.5f * height, -0.5f * depth), Vector2(1, 0), Vector3(0, -1, 0), Vector3(1, 0, 0)); // 7

        // back
        vertices.emplace_back(Vector3(-0.5f * width, -0.5f * height,  0.5f * depth), Vector2(1, 1), Vector3(0, 0, 1), Vector3(0, 1, 0)); // 8
        vertices.emplace_back(Vector3(-0.5f * width,  0.5f * height,  0.5f * depth), Vector2(1, 0), Vector3(0, 0, 1), Vector3(0, 1, 0)); // 9
        vertices.emplace_back(Vector3( 0.5f * width, -0.5f * height,  0.5f * depth), Vector2(0, 1), Vector3(0, 0, 1), Vector3(0, 1, 0)); // 10
        vertices.emplace_back(Vector3( 0.5f * width,  0.5f * height,  0.5f * depth), Vector2(0, 0), Vector3(0, 0, 1), Vector3(0, 1, 0)); // 11

        // top
        vertices.emplace_back(Vector3(-0.5f * width,  0.5f * height,  0.5f * depth), Vector2(0, 0), Vector3(0, 1, 0), Vector3(1, 0, 0)); // 12
        vertices.emplace_back(Vector3(-0.5f * width,  0.5f * height, -0.5f * depth), Vector2(0, 1), Vector3(0, 1, 0), Vector3(1, 0, 0)); // 13
        vertices.emplace_back(Vector3( 0.5f * width,  0.5f * height,  0.5f * depth), Vector2(1, 0), Vector3(0, 1, 0), Vector3(1, 0, 0)); // 14
        vertices.emplace_back(Vector3( 0.5f * width,  0.5f * height, -0.5f * depth), Vector2(1, 1), Vector3(0, 1, 0), Vector3(1, 0, 0)); // 15

        // left
        vertices.emplace_back(Vector3(-0.5f * width, -0.5f * height,  0.5f * depth), Vector2(0, 1), Vector3(-1, 0, 0), Vector3(0, 1, 0)); // 16
        vertices.emplace_back(Vector3(-0.5f * width,  0.5f * height,  0.5f * depth), Vector2(0, 0), Vector3(-1, 0, 0), Vector3(0, 1, 0)); // 17
        vertices.emplace_back(Vector3(-0.5f * width, -0.5f * height, -0.5f * depth), Vector2(1, 1), Vector3(-1, 0, 0), Vector3(0, 1, 0)); // 18
        vertices.emplace_back(Vector3(-0.5f * width,  0.5f * height, -0.5f * depth), Vector2(1, 0), Vector3(-1, 0, 0), Vector3(0, 1, 0)); // 19

        // right
        vertices.emplace_back(Vector3( 0.5f * width, -0.5f * height,  0.5f * depth), Vector2(1, 1), Vector3(1, 0, 0), Vector3(0, 1, 0)); // 20
        vertices.emplace_back(Vector3( 0.5f * width,  0.5f * height,  0.5f * depth), Vector2(1, 0), Vector3(1, 0, 0), Vector3(0, 1, 0)); // 21
        vertices.emplace_back(Vector3( 0.5f * width, -0.5f * height, -0.5f * depth), Vector2(0, 1), Vector3(1, 0, 0), Vector3(0, 1, 0)); // 22
        vertices.emplace_back(Vector3( 0.5f * width,  0.5f * height, -0.5f * depth), Vector2(0, 0), Vector3(1, 0, 0), Vector3(0, 1, 0)); // 23

        // front
        indices.emplace_back(0); indices.emplace_back(1); indices.emplace_back(2);
        indices.emplace_back(2); indices.emplace_back(1); indices.emplace_back(3);

        // bottom
        indices.emplace_back(4); indices.emplace_back(5); indices.emplace_back(6);
        indices.emplace_back(6); indices.emplace_back(5); indices.emplace_back(7);

        // back
        indices.emplace_back(10); indices.emplace_back(9); indices.emplace_back(8);
        indices.emplace_back(11); indices.emplace_back(9); indices.emplace_back(10);

        // top
        indices.emplace_back(14); indices.emplace_back(13); indices.emplace_back(12);
        indices.emplace_back(15); indices.emplace_back(13); indices.emplace_back(14);

        // left
        indices.emplace_back(16); indices.emplace_back(17); indices.emplace_back(18);
        indices.emplace_back(18); indices.emplace_back(17); indices.emplace_back(19);

        // right
        indices.emplace_back(22); indices.emplace_back(21); indices.emplace_back(20);
        indices.emplace_back(23); indices.emplace_back(21); indices.emplace_back(22);

        // clang-format on
    }

    static void generateSphere(std::vector<RHIVertexPosUvNrmTan>& vertices,
                               std::vector<std::uint32_t>& indices,
                               float radius = 0.5f, std::uint32_t segments = 32,
                               std::uint32_t rings = 16)
    {
        using worse::math::Vector2;
        using worse::math::Vector3;

        vertices.clear();
        indices.clear();

        // +2 for the top and bottom pole vertices
        vertices.reserve((segments + 1) * (rings - 1) + 2);
        indices.reserve(segments * rings * 6);

        // --- Vertices ---

        // Top pole
        vertices.emplace_back(Vector3(0.0f, radius, 0.0f),
                              Vector2(0.5f, 0.0f),
                              Vector3(0.0f, 1.0f, 0.0f),
                              Vector3(1.0f, 0.0f, 0.0f));
        const std::uint32_t topPoleIndex = 0;

        // Rings (excluding poles)
        for (std::uint32_t r = 1; r < rings; ++r)
        {
            float phi =
                math::PI * static_cast<float>(r) / static_cast<float>(rings);
            float sinPhi = std::sin(phi);
            float cosPhi = std::cos(phi);

            for (std::uint32_t s = 0; s <= segments; ++s)
            {
                float theta = 2.0f * math::PI * static_cast<float>(s) /
                              static_cast<float>(segments);
                float sinTheta = std::sin(theta);
                float cosTheta = std::cos(theta);

                Vector3 position(radius * sinPhi * cosTheta,
                                 radius * cosPhi,
                                 radius * sinPhi * sinTheta);
                Vector2 uv(static_cast<float>(s) / static_cast<float>(segments),
                           static_cast<float>(r) / static_cast<float>(rings));
                Vector3 normal = normalize(position);
                // A robust tangent calculation
                Vector3 tangent = normalize(Vector3(-sinTheta, 0.0f, cosTheta));

                vertices.emplace_back(position, uv, normal, tangent);
            }
        }

        // Bottom pole
        vertices.emplace_back(Vector3(0.0f, -radius, 0.0f),
                              Vector2(0.5f, 1.0f),
                              Vector3(0.0f, -1.0f, 0.0f),
                              Vector3(1.0f, 0.0f, 0.0f));
        const std::uint32_t bottomPoleIndex =
            static_cast<std::uint32_t>(vertices.size() - 1);

        // --- Indices ---

        // Top fan
        for (std::uint32_t s = 0; s < segments; ++s)
        {
            // vertex index for the first ring starts at 1 (after the top pole)
            std::uint32_t first  = 1 + s;
            std::uint32_t second = 1 + s + 1;
            indices.emplace_back(topPoleIndex);
            indices.emplace_back(second);
            indices.emplace_back(first);
        }

        // Middle quads
        for (std::uint32_t r = 0; r < rings - 2; ++r)
        {
            for (std::uint32_t s = 0; s < segments; ++s)
            {
                // vertex index for the first ring starts at 1
                std::uint32_t first_row_start  = 1 + r * (segments + 1);
                std::uint32_t second_row_start = 1 + (r + 1) * (segments + 1);

                std::uint32_t i0 = first_row_start + s;
                std::uint32_t i1 = first_row_start + s + 1;
                std::uint32_t i2 = second_row_start + s;
                std::uint32_t i3 = second_row_start + s + 1;

                indices.emplace_back(i0);
                indices.emplace_back(i2);
                indices.emplace_back(i1);

                indices.emplace_back(i1);
                indices.emplace_back(i2);
                indices.emplace_back(i3);
            }
        }

        // Bottom fan
        // vertex index for the start of the last ring
        std::uint32_t lastRingStartIndex = 1 + (rings - 2) * (segments + 1);
        for (std::uint32_t s = 0; s < segments; ++s)
        {
            std::uint32_t first  = lastRingStartIndex + s;
            std::uint32_t second = lastRingStartIndex + s + 1;
            indices.emplace_back(bottomPoleIndex);
            indices.emplace_back(first);
            indices.emplace_back(second);
        }
    }

    static void generateCylinder(std::vector<RHIVertexPosUvNrmTan>& vertices,
                                 std::vector<std::uint32_t>& indices,
                                 float radius = 0.5f, float height = 1.0f,
                                 std::uint32_t segments       = 32,
                                 std::uint32_t heightSegments = 1,
                                 bool topCap = true, bool bottomCap = true)
    {
        using worse::math::Vector2;
        using worse::math::Vector3;

        vertices.clear();
        indices.clear();

        // 计算顶点数量：侧面 + 顶面 + 底面
        std::uint32_t sideVertices = (segments + 1) * (heightSegments + 1);
        std::uint32_t capVertices =
            topCap ? (segments + 2) : 0; // 中心点 + 边缘点
        capVertices += bottomCap ? (segments + 2) : 0;

        vertices.reserve(sideVertices + capVertices);

        // 计算索引数量
        std::uint32_t sideIndices = segments * heightSegments * 6;
        std::uint32_t capIndices =
            (topCap ? segments * 3 : 0) + (bottomCap ? segments * 3 : 0);
        indices.reserve(sideIndices + capIndices);

        float halfHeight = height * 0.5f;

        // 生成侧面顶点
        for (std::uint32_t h = 0; h <= heightSegments; ++h)
        {
            float y = -halfHeight + (height * static_cast<float>(h) /
                                     static_cast<float>(heightSegments));
            float v =
                static_cast<float>(h) / static_cast<float>(heightSegments);

            for (std::uint32_t s = 0; s <= segments; ++s)
            {
                float theta = 2.0f * math::PI * static_cast<float>(s) /
                              static_cast<float>(segments);
                float x = radius * std::cos(theta);
                float z = radius * std::sin(theta);
                float u = static_cast<float>(s) / static_cast<float>(segments);

                Vector3 position(x, y, z);
                Vector2 uv(u, v);
                Vector3 normal = normalize(Vector3(x, 0.0f, z)); // 径向法线
                Vector3 tangent =
                    normalize(Vector3(-std::sin(theta), 0.0f, std::cos(theta)));

                vertices.emplace_back(position, uv, normal, tangent);
            }
        }

        // 生成侧面索引
        for (std::uint32_t h = 0; h < heightSegments; ++h)
        {
            for (std::uint32_t s = 0; s < segments; ++s)
            {
                std::uint32_t first  = h * (segments + 1) + s;
                std::uint32_t second = first + segments + 1;

                indices.emplace_back(first);
                indices.emplace_back(second);
                indices.emplace_back(first + 1);

                indices.emplace_back(second);
                indices.emplace_back(second + 1);
                indices.emplace_back(first + 1);
            }
        }

        std::uint32_t currentVertexIndex = sideVertices;

        // 生成顶盖
        if (topCap)
        {
            // 顶盖中心点
            Vector3 topCenter(0.0f, halfHeight, 0.0f);
            Vector2 topCenterUv(0.5f, 0.5f);
            Vector3 topNormal(0.0f, 1.0f, 0.0f);
            Vector3 topTangent(1.0f, 0.0f, 0.0f);

            std::uint32_t topCenterIndex = currentVertexIndex;
            vertices.emplace_back(topCenter,
                                  topCenterUv,
                                  topNormal,
                                  topTangent);
            currentVertexIndex++;

            // 顶盖边缘点
            for (std::uint32_t s = 0; s <= segments; ++s)
            {
                float theta = 2.0f * math::PI * static_cast<float>(s) /
                              static_cast<float>(segments);
                float x = radius * std::cos(theta);
                float z = radius * std::sin(theta);

                Vector3 position(x, halfHeight, z);
                Vector2 uv(0.5f + 0.5f * std::cos(theta),
                           0.5f + 0.5f * std::sin(theta));
                Vector3 normal(0.0f, 1.0f, 0.0f);
                Vector3 tangent(1.0f, 0.0f, 0.0f);

                vertices.emplace_back(position, uv, normal, tangent);
            }

            // 顶盖索引
            for (std::uint32_t s = 0; s < segments; ++s)
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
            Vector3 bottomTangent(1.0f, 0.0f, 0.0f);

            std::uint32_t bottomCenterIndex = currentVertexIndex;
            vertices.emplace_back(bottomCenter,
                                  bottomCenterUv,
                                  bottomNormal,
                                  bottomTangent);
            currentVertexIndex++;

            // 底盖边缘点
            for (std::uint32_t s = 0; s <= segments; ++s)
            {
                float theta = 2.0f * math::PI * static_cast<float>(s) /
                              static_cast<float>(segments);
                float x = radius * std::cos(theta);
                float z = radius * std::sin(theta);

                Vector3 position(x, -halfHeight, z);
                Vector2 uv(0.5f + 0.5f * std::cos(theta),
                           0.5f - 0.5f * std::sin(theta));
                Vector3 normal(0.0f, -1.0f, 0.0f);
                Vector3 tangent(1.0f, 0.0f, 0.0f);

                vertices.emplace_back(position, uv, normal, tangent);
            }

            // 底盖索引 (逆时针)
            for (std::uint32_t s = 0; s < segments; ++s)
            {
                indices.emplace_back(bottomCenterIndex);
                indices.emplace_back(currentVertexIndex + s + 1);
                indices.emplace_back(currentVertexIndex + s);
            }
        }
    }

    static void generateCapsule(std::vector<RHIVertexPosUvNrmTan>& vertices,
                                std::vector<std::uint32_t>& indices,
                                float radius = 0.5f, float height = 2.0f,
                                std::uint32_t segments   = 32,
                                std::uint32_t totalRings = 16)
    {
        using worse::math::Vector2;
        using worse::math::Vector3;

        vertices.clear();
        indices.clear();

        const float cylinderHeight     = std::max(0.0f, height - 2.0f * radius);
        const float halfCylinderHeight = cylinderHeight * 0.5f;
        const std::uint32_t hemisphereRings = totalRings / 2;

        // --- Arc-length calculation for proportional V coordinate ---
        const float hemisphereArcLength = math::PI * 0.5f * radius;
        const float totalArcLength =
            2.0f * hemisphereArcLength + cylinderHeight;

        // --- Vertices ---
        // A single loop to generate all vertices seamlessly
        // We add 3 rings: top pole, junction between top cap and cylinder,
        // junction between cylinder and bottom cap, bottom pole
        std::uint32_t rings =
            hemisphereRings * 2 + 1; // total rings for the whole shape
        if (cylinderHeight > 0)
            rings += 1; // an extra ring for the cylinder middle if it exists

        for (std::uint32_t r = 0; r <= rings; ++r)
        {
            float v_ratio = static_cast<float>(r) / static_cast<float>(rings);
            float currentArcDist = v_ratio * totalArcLength;

            float phi, y, effectiveRadius;
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
                float arcDist_from_bottom = totalArcLength - currentArcDist;
                phi                       = arcDist_from_bottom / radius;
                effectiveRadius           = radius * std::sin(phi);
                y            = -(radius * std::cos(phi) + halfCylinderHeight);
                normal_y_dir = Vector3(0, -std::cos(phi), 0);
            }
            else // Cylinder body
            {
                effectiveRadius = radius;
                y = halfCylinderHeight - (currentArcDist - hemisphereArcLength);
                normal_y_dir = Vector3(0, 0, 0);
            }

            for (std::uint32_t s = 0; s <= segments; ++s)
            {
                float u = static_cast<float>(s) / static_cast<float>(segments);
                float theta    = 2.0f * math::PI * u;
                float cosTheta = std::cos(theta);
                float sinTheta = std::sin(theta);

                float x = effectiveRadius * cosTheta;
                float z = effectiveRadius * sinTheta;

                Vector3 position(x, y, z);
                Vector2 uv(u, v_ratio);
                Vector3 normal =
                    normalize(Vector3(x, 0.0f, z) + normal_y_dir * radius);
                Vector3 tangent = normalize(Vector3(-sinTheta, 0.0f, cosTheta));

                vertices.emplace_back(position, uv, normal, tangent);
            }
        }

        // --- Indices ---
        for (std::uint32_t r = 0; r < rings; ++r)
        {
            for (std::uint32_t s = 0; s < segments; ++s)
            {
                std::uint32_t i0 = r * (segments + 1) + s;
                std::uint32_t i1 = i0 + 1;
                std::uint32_t i2 = (r + 1) * (segments + 1) + s;
                std::uint32_t i3 = i2 + 1;

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