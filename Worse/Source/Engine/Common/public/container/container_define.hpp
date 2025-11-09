#pragma once

namespace Worse
{

    // Static tags for container construction

    // Not to initialize memory
    enum ENoInit
    {
        NoInit
    };
    // Initialize memory in place
    enum EInPlace
    {
        InPlace
    };
    // Initialize each element
    enum EPerElement
    {
        PerElement
    };

} // namespace Worse