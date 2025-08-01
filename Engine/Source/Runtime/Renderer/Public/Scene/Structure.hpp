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

        // Update children transform
        void refreshTransform(math::Matrix4 const& parent_transform);
    };

} // namespace worse