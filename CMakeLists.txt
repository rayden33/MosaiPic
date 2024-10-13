cmake_minimum_required(VERSION 3.0)
project(MosPic)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)

add_executable(${PROJECT_NAME} src/mosaipic.cpp)
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})