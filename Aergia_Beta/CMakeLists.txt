cmake_minimum_required(VERSION 2.8.4)
project(Aergia)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g")

set(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/build/")
set(LIBRARY_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/build/")

set(SOURCE_FILES
    src/io/GraphicsDisplay.cpp
    src/io/GraphicsDisplay.h
    src/math/numericalTypes.h
    src/math/Transform.cpp
    src/math/Transform.h
    src/scene/Camera.h
    src/scene/Camera.cpp
    src/scene/CameraController.h
    src/scene/CameraController.cpp
    src/graphics/OpenGL.h
    src/graphics/OpenGL.cpp
    )

set(HEADERS
    io/GraphicsDisplay.h
    math/numericalTypes.h
    math/Transform.h
    scene/Camera.h
    scene/CameraController.h
    graphics/OpenGL.h
    Aergia.h
    )

include_directories(${PROJECT_SOURCE_DIR}/src)

add_library(Aergia STATIC ${SOURCE_FILES})

find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
include_directories(${GLFW_INCLUDE_DIRS})

include_directories (${OPENGL_INCLUDE_DIR})
set(LIBRARIES GL GLU glut X11 Xxf86vm Xrandr pthread Xi m Xext GLEW Xmu freetype ftgl)

target_link_libraries(Aergia ${GLFW_LIBRARIES})
target_link_libraries(Aergia ${GLFW_STATIC_LIBRARIES})

target_link_libraries (Aergia ${LIBRARIES})
target_link_libraries(Aergia /usr/local/lib/libSOIL.a)

#INSTALL(TARGETS Aergia
#        DESTINATION "lib"
#       )

foreach(HEADER ${HEADERS})
   string(REGEX MATCH "(.*)[/\\]" DIR ${HEADER})
   install(FILES src/${HEADER} DESTINATION ${LIBRARY_OUTPUT_PATH}/include/aergia/${DIR})
endforeach(HEADER ${HEADERS})
       

