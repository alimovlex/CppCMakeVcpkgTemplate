# CMakeLists.txt
cmake_minimum_required(VERSION 3.15)
project(cpp_template)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 17)
if(NOT APPLE)
    # set(CMAKE_CXX_CLANG_TIDY clang-tidy)
endif()
include_directories(/opt/local/include)

# Find dependencies provided by vcpkg (via vcpkg.cmake)
find_package(SDL2 CONFIG REQUIRED)
find_package(GLEW CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
#find_package(FreeImage CONFIG REQUIRED)
#find_package(fmt CONFIG REQUIRED)

set(
    CPP_TEMPLATE_SOURCES
    src/main.c
    src/MathLib.c
        src/config.c
        src/file.c
        src/maze.c
        src/noise.c
        src/walker.c
        src/camera.c
        src/drawer.c
        src/mesh.c
        src/scene.c
        src/window.c
)
set(
    CPP_TEMPLATE_TEST_SOURCES
    src/test.cpp
)

# main target
add_executable(main ${CPP_TEMPLATE_SOURCES})
target_link_libraries(main PRIVATE SDL2::SDL2 GLEW::GLEW /opt/local/lib/libfreeimage.dylib)
find_library(COREFOUNDATION_FRAMEWORK CoreFoundation)
target_link_libraries(main PRIVATE ${COREFOUNDATION_FRAMEWORK})
set_property(TARGET main PROPERTY CXX_STANDARD 17)

# tests target
add_executable(tests ${CPP_TEMPLATE_TEST_SOURCES})
target_link_libraries(tests PRIVATE SDL2::SDL2 GLEW::GLEW /opt/local/lib/libfreeimage.dylib glfw)
#set_property(TARGET tests PROPERTY CXX_STANDARD 17)
enable_testing()
add_test(tests tests)

add_custom_target(
    check-format
    COMMAND clang-format --dry-run ${CPP_TEMPLATE_SOURCES} ${CPP_TEMPLATE_TEST_SOURCES}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "checking format in src..."
)

add_custom_target(
    format
    COMMAND clang-format -i ${CPP_TEMPLATE_SOURCES} ${CPP_TEMPLATE_TEST_SOURCES}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "checking format in src..."
)
