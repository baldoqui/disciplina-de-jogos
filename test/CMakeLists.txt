cmake_minimum_required(VERSION 3.16)
project(disciplina_de_jogos_tests LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(GTest REQUIRED)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../src)

add_executable(tests
  Vector2DTest.cpp
  Transform2DTest.cpp
  ${SRC_DIR}/Vector2D.cpp
  ${SRC_DIR}/Transform2D.cpp
)
target_include_directories(tests PRIVATE ${SRC_DIR})
target_compile_options(tests PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(tests PRIVATE GTest::gtest_main)

enable_testing()
include(GoogleTest)
gtest_discover_tests(tests)
