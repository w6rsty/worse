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
        Max
    };

    static void generateQuad3D(std::vector<RHIVertexPosUvNrmTan>& vertices,
                               std::vector<std::uint32_t>& indices)
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

        vertices.emplace_back(Vector3(-0.5f,  0.0f,  0.5f), Vector2(0, 0), Vector3(0, 1, 0), Vector3(1, 0, 0)); // 0 top-left
        vertices.emplace_back(Vector3( 0.5f,  0.0f,  0.5f), Vector2(1, 0), Vector3(0, 1, 0), Vector3(1, 0, 0)); // 1 top-right
        vertices.emplace_back(Vector3(-0.5f,  0.0f, -0.5f), Vector2(0, 1), Vector3(0, 1, 0), Vector3(1, 0, 0)); // 2 bottom-left
        vertices.emplace_back(Vector3( 0.5f,  0.0f, -0.5f), Vector2(1, 1), Vector3(0, 1, 0), Vector3(1, 0, 0)); // 3 bottom-right

        indices.emplace_back(3);
        indices.emplace_back(2);
        indices.emplace_back(0);
        indices.emplace_back(3);
        indices.emplace_back(0);
        indices.emplace_back(1);
        // clang-format on
    }

    static void generateCube(std::vector<RHIVertexPosUvNrmTan>& vertices,
                             std::vector<std::uint32_t>& indices)
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
        vertices.emplace_back(Vector3(-0.5f, -0.5f, -0.5f), Vector2(0, 1), Vector3(0, 0, -1), Vector3(0, 1, 0)); // 0
        vertices.emplace_back(Vector3(-0.5f,  0.5f, -0.5f), Vector2(0, 0), Vector3(0, 0, -1), Vector3(0, 1, 0)); // 1
        vertices.emplace_back(Vector3( 0.5f, -0.5f, -0.5f), Vector2(1, 1), Vector3(0, 0, -1), Vector3(0, 1, 0)); // 2
        vertices.emplace_back(Vector3( 0.5f,  0.5f, -0.5f), Vector2(1, 0), Vector3(0, 0, -1), Vector3(0, 1, 0)); // 3

        // bottom
        vertices.emplace_back(Vector3(-0.5f, -0.5f,  0.5f), Vector2(0, 1), Vector3(0, -1, 0), Vector3(1, 0, 0)); // 4
        vertices.emplace_back(Vector3(-0.5f, -0.5f, -0.5f), Vector2(0, 0), Vector3(0, -1, 0), Vector3(1, 0, 0)); // 5
        vertices.emplace_back(Vector3( 0.5f, -0.5f,  0.5f), Vector2(1, 1), Vector3(0, -1, 0), Vector3(1, 0, 0)); // 6
        vertices.emplace_back(Vector3( 0.5f, -0.5f, -0.5f), Vector2(1, 0), Vector3(0, -1, 0), Vector3(1, 0, 0)); // 7

        // back
        vertices.emplace_back(Vector3(-0.5f, -0.5f,  0.5f), Vector2(1, 1), Vector3(0, 0, 1), Vector3(0, 1, 0)); // 8
        vertices.emplace_back(Vector3(-0.5f,  0.5f,  0.5f), Vector2(1, 0), Vector3(0, 0, 1), Vector3(0, 1, 0)); // 9
        vertices.emplace_back(Vector3( 0.5f, -0.5f,  0.5f), Vector2(0, 1), Vector3(0, 0, 1), Vector3(0, 1, 0)); // 10
        vertices.emplace_back(Vector3( 0.5f,  0.5f,  0.5f), Vector2(0, 0), Vector3(0, 0, 1), Vector3(0, 1, 0)); // 11

        // top
        vertices.emplace_back(Vector3(-0.5f,  0.5f,  0.5f), Vector2(0, 0), Vector3(0, 1, 0), Vector3(1, 0, 0)); // 12
        vertices.emplace_back(Vector3(-0.5f,  0.5f, -0.5f), Vector2(0, 1), Vector3(0, 1, 0), Vector3(1, 0, 0)); // 13
        vertices.emplace_back(Vector3( 0.5f,  0.5f,  0.5f), Vector2(1, 0), Vector3(0, 1, 0), Vector3(1, 0, 0)); // 14
        vertices.emplace_back(Vector3( 0.5f,  0.5f, -0.5f), Vector2(1, 1), Vector3(0, 1, 0), Vector3(1, 0, 0)); // 15

        // left
        vertices.emplace_back(Vector3(-0.5f, -0.5f,  0.5f), Vector2(0, 1), Vector3(-1, 0, 0), Vector3(0, 1, 0)); // 16
        vertices.emplace_back(Vector3(-0.5f,  0.5f,  0.5f), Vector2(0, 0), Vector3(-1, 0, 0), Vector3(0, 1, 0)); // 17
        vertices.emplace_back(Vector3(-0.5f, -0.5f, -0.5f), Vector2(1, 1), Vector3(-1, 0, 0), Vector3(0, 1, 0)); // 18
        vertices.emplace_back(Vector3(-0.5f,  0.5f, -0.5f), Vector2(1, 0), Vector3(-1, 0, 0), Vector3(0, 1, 0)); // 19

        // right
        vertices.emplace_back(Vector3( 0.5f, -0.5f,  0.5f), Vector2(1, 1), Vector3(1, 0, 0), Vector3(0, 1, 0)); // 20
        vertices.emplace_back(Vector3( 0.5f,  0.5f,  0.5f), Vector2(1, 0), Vector3(1, 0, 0), Vector3(0, 1, 0)); // 21
        vertices.emplace_back(Vector3( 0.5f, -0.5f, -0.5f), Vector2(0, 1), Vector3(1, 0, 0), Vector3(0, 1, 0)); // 22
        vertices.emplace_back(Vector3( 0.5f,  0.5f, -0.5f), Vector2(0, 0), Vector3(1, 0, 0), Vector3(0, 1, 0)); // 23

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

} // namespace worse::geometry