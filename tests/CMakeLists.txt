cmake_minimum_required(VERSION 3.8)

set(This 8080_tests)

set(Sources
  test_8080.cpp
)

add_executable(${This} ${Sources})
target_link_libraries(${This} PUBLIC
  gtest_main
)

add_test(
  NAME ${This}
  COMMAND ${This}
)