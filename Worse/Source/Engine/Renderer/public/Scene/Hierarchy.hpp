#pragma once
#include "Math/Math.hpp"
#include "../Renderable.hpp"

#include <vector>

namespace worse
{

    struct Node : public IRenderable
    {
        std::weak_ptr<Node> parent;
        std::vector<std::shared_ptr<Node>> children;
        math::Matrix4 localTransform;
        math::Matrix4 worldTransform;

        void draw(math::Matrix4 const& topMat, DrawContext& ctx) override
        {
            for (std::shared_ptr<Node> const& child : children)
            {
                child->draw(topMat, ctx);
            }
        }

        /**
         * @brief 更新自身和子节点的变换矩阵
         */
        void refreshTransform(math::Matrix4 const& parentTransform);
    };

} // namespace worse