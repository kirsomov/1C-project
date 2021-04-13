cmake_minimum_required(VERSION 3.19)
project(1c_project)

set(CMAKE_CXX_STANDARD 14)
find_package(OpenCV REQUIRED)

add_executable(1c_project main.cpp)
#set_property(TARGET 1c_project PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "/usr/include/opencv4")
target_link_libraries(1c_project ${OpenCV_LIBS})
