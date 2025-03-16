add_executable(8080Emulator main.cpp)
add_subdirectory(CPU)
target_link_libraries(8080Emulator PRIVATE keys 8080 screen logger)