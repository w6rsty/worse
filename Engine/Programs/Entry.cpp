#include "Log.hpp"
#include "Application.hpp"

int main(int argc, char** argv)
{
    worse::Logger::initialize();

    worse::pc::Application app(argc, argv);
    app.run();

    worse::Logger::shutdown();
}