cmake_minimum_required(VERSION 3.25)
project(GC_Proyecto3_Raytracing)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SDL2_INCLUDE_DIR "C:/Libs/SDL/include")
set(SDL2_LIB_DIR "C:/Libs/SDL/lib/x64")

# Agregar la ruta de inclusión de la biblioteca glm
set(GLM_INCLUDE_DIR "C:/Users/50242/Documents/Universidad/TercerAño/GraficasComputadora/GC-Proyecto3-Raytracing/glm")

# Utiliza GLM_INCLUDE_DIR en lugar de CMAKE_INCLUDE_PATH
include_directories(${SDL2_INCLUDE_DIR} ${GLM_INCLUDE_DIR})
link_directories(${SDL2_LIB_DIR})

add_executable(GC_Proyecto3_Raytracing main.cpp camera.cpp print.h cube.cpp cube.h)
target_link_libraries(GC_Proyecto3_Raytracing SDL2main SDL2)
