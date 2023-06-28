cmake_minimum_required(VERSION 3.25)
project(AirMesh)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)
include_directories(gl)
include_directories(/opt/X11/include)

link_libraries(gl)
link_libraries(glu)
link_libraries(glut)
link_directories(/opt/X11/lib)

add_executable(AirMesh
        air_mesh.cpp
        air_mesh.h
        main.cpp
        shape.cpp
        shape.h
        triangle.cpp
        triangle.h
        triangle_half_edge.cpp
        triangle_half_edge.h)
