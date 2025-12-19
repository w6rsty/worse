#pragma once
#include "math/math.hpp"
#include "../Renderable.hpp"

#include <vector>

namespace Worse
{

    struct Node : public IRenderable
    {
        std::weak_ptr<Node> parent;
        std::vector<std::shared_ptr<Node>> children;
        Matrix4 localTransform;
        Matrix4 worldTransform;

        void draw(Matrix4 const& topMat, DrawContext& ctx) override
        {
            for (std::shared_ptr<Node> const& child : children)
            {
                child->draw(topMat, ctx);
            }
        }

        /**
         * @brief 更新自身和子节点的变换矩阵
         */
        void refreshTransform(Matrix4 const& parentTransform);
    };

} // namespace Worse