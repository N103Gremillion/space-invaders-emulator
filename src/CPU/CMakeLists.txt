find_package(SDL2 REQUIRED)

add_library(keys keys.cpp)
add_library(8080 8080.cpp)
add_library(screen screen.cpp)
add_library(logger logger.cpp)

target_include_directories(keys PUBLIC include)
target_include_directories(8080 PUBLIC include)
target_include_directories(screen PUBLIC include)
target_include_directories(logger PUBLIC include)

target_include_directories(screen PRIVATE ${SDL2_INCLUDE_DIRS})

target_link_libraries(screen PRIVATE SDL2::SDL2)

target_link_libraries(8080Emulator PRIVATE keys 8080 screen logger)