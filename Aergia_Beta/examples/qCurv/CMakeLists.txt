cmake_minimum_required(VERSION 2.8.4)
project(qCurv)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -g")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${EXECUTABLE_OUTPUT_PATH}/qCurv")

include_directories(${AERGIA_INCLUDE_DIR})
include_directories(${AERGIA_INCLUDE_DIR}/aergia)
 
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
include_directories(${GLFW_INCLUDE_DIRS})

include_directories(/usr/include/eigen3/)

#include_directories (${OPENGL_INCLUDE_DIR})
#set(LIBRARIES GL GLU glut X11 Xxf86vm Xrandr pthread Xi m Xext GLEW Xmu freetype ftgl)

set(SOURCE_FILES 
		main.cpp
		UGrid.h
)

add_executable(qCurv ${SOURCE_FILES})
#target_link_libraries(qCurv LINK_PUBLIC Aergia)

target_link_libraries(qCurv ${AERGIA_LIB})
target_link_libraries(qCurv ${GLFW_LIBRARIES})
target_link_libraries(qCurv ${GLFW_STATIC_LIBRARIES})