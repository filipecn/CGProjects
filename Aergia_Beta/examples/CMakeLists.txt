cmake_minimum_required(VERSION 2.8.4)
project(examples)

set(EXECUTABLE_OUTPUT_PATH 
"/home/filipecn/Documents/CGProjects/Aergia_Beta/build/examples"
)

set(AERGIA_INCLUDE_DIR 
"/home/filipecn/Documents/CGProjects/Aergia_Beta/build/include/"
)

set(AERGIA_LIB
"/home/filipecn/Documents/CGProjects/Aergia_Beta/build/libAergia.a"
)

add_subdirectory (helloAergia)
add_subdirectory (qCurv)