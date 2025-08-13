#include "Scene/Hierarchy.hpp"

namespace worse
{

    void Node::refreshTransform(math::Matrix4 const& parentTransform)
    {
        worldTransform = parentTransform * localTransform;

        for (auto& child : children)
        {
            child->refreshTransform(worldTransform);
        }
    }

} // namespace worse