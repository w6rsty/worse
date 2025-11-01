#include "Scene/Hierarchy.hpp"

namespace Worse
{

    void Node::refreshTransform(math::Matrix4 const& parentTransform)
    {
        worldTransform = parentTransform * localTransform;

        for (auto& child : children)
        {
            child->refreshTransform(worldTransform);
        }
    }

} // namespace Worse