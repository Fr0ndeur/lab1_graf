cmake_minimum_required(VERSION 3.15)
project(OpenGL_Project)

set(CMAKE_CXX_STANDARD 17)

# Шляхи до бібліотек
include_directories(include)

# GLFW
add_subdirectory(external/glfw)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/include/glm)


# Головний файл
add_executable(OpenGL_Project src/main.cpp src/glad.c)

# Лінкування
target_link_libraries(OpenGL_Project glfw)
