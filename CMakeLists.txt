cmake_minimum_required(VERSION 3.8)

set(This Space_Invaders_Emulator)

project(${This} CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

enable_testing()

find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)

include_directories(${This} ${SDL2_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS})

add_subdirectory(googletest)

set (Headers
  ./src/CPU/8080.hpp
  ./src/CPU/keys.hpp
  ./src/CPU/logger.hpp
  ./src/CPU/Screen.hpp
  ./src/CPU/Registers.hpp
)

set(Sources
  ./src/CPU/8080.cpp
  ./src/CPU/keys.cpp
  ./src/CPU/logger.cpp
  ./src/CPU/Screen.cpp
  ./src/CPU/Registers.cpp
  ./src/main.cpp
)

# Add an executable (main entry point)
add_executable(${This} ${Sources})

target_link_libraries(${This} ${SDL2_LIBRARIES} SDL2 SDL2_ttf)
target_link_libraries(${This} )
# keep test separate from code
add_subdirectory(tests)

