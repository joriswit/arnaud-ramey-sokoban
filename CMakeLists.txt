project(sokoban)
cmake_minimum_required(VERSION 2.8)
include(GenerateExportHeader)
# Debug, Release, RelWithDebInfo and MinSizeRe
set(CMAKE_BUILD_TYPE RelWithDebInfo)
SET(CMAKE_VERBOSE_MAKEFILE ON)
if(CMAKE_COMPILER_IS_GNUCXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra") # add extra warnings
endif()

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

if(WIN32)
  add_library(arnaud-ramey-sokoban SHARED
    Plugin.cpp
    arnaud-ramey-sokoban.def
  )
  target_link_libraries(arnaud-ramey-sokoban sokoban)
  generate_export_header(arnaud-ramey-sokoban)
endif()
