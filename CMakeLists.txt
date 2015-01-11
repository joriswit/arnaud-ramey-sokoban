cmake_minimum_required(VERSION 2.8)
project(sokoban)
# Debug, Release, RelWithDebInfo and MinSizeRe
set(CMAKE_BUILD_TYPE Debug)
find_package(OpenCV REQUIRED)
find_package(X11 REQUIRED)
find_package(Threads REQUIRED)
include_directories(libs)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR} ${PROJECT_BINARY_DIR})
CONFIGURE_FILE("${PROJECT_SOURCE_DIR}/sokoban_path.h.in"
               "${PROJECT_BINARY_DIR}/sokoban_path.h")

add_library(sokoban
  Situation.cpp
  SituationTree.cpp
  Solver.cpp
  Movement.cpp
  IO/IO.cpp)

add_executable(test_sokoban test.cpp)
target_link_libraries(test_sokoban sokoban)
add_executable(test_sokoban_img test_img.cpp GUI.cpp)
target_link_libraries(test_sokoban_img sokoban ${OpenCV_LIBS} ${X11_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
