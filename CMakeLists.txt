cmake_minimum_required(VERSION 3.8)

set(This Space_Invaders_Emulator)

project(${This} CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

enable_testing()

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
)

add_library(${This} STATIC ${Headers} ${Sources})

# keep test separate from code
add_subdirectory(tests)

