cmake_minimum_required(VERSION 3.10)
project(Projekt-Grafika-Komputerowa)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set the executable name
set(EXECUTABLE_NAME grafika_app)

# Allow CMake to find Homebrew packages on Apple Silicon (M1/M2) and Intel Macs
if(APPLE)
    set(CMAKE_FIND_FRAMEWORK FIRST)
    list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew" "/usr/local")
endif()

# Find necessary packages
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)

# Gather all source files
file(GLOB SOURCES 
    "src/*.cpp" 
    "src/*.h"
    "src/*.hpp"
)

# Add executable
add_executable(${EXECUTABLE_NAME} ${SOURCES})

# Include directories
target_include_directories(${EXECUTABLE_NAME} PRIVATE "src")

# Link libraries
target_link_libraries(${EXECUTABLE_NAME} PRIVATE
    OpenGL::GL
    glfw
    GLEW::GLEW
    glm::glm
)
