#pragma once

namespace worse
{

    class Engine
    {
    public:
        static void initialize();
        static void shutdown();
        static void tick();
    };

} // namespace worse