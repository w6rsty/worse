file(GLOB_RECURSE GAME_SOURCE
    Entry.cpp
)

file(GLOB_RECURSE GAME_HEADERS
    World.hpp
)

add_executable(Game
    ${GAME_SOURCE}
    ${GAME_HEADERS}
)

target_link_libraries(Game PRIVATE
    Worse::Engine
    Worse::Renderer
    Worse::ECS
)