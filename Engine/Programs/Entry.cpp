#include "Log.hpp"
#include "Application/Application.hpp"

int main(int argc, char** argv)
{
    worse::Logger::initialize();

    Application app(argc, argv);
    app.run();

    worse::Logger::shutdown();
}