cmake_minimum_required(VERSION 3.0.0)
project(svo-renderer VERSION 1.0)

add_definitions("-std=c++11")

include(${CMAKE_BINARY_DIR}/conanbuildinfo_multi.cmake)
conan_basic_setup()

find_package(Vulkan REQUIRED)

add_executable(svo-renderer
    main.cpp
    Engine.h
    Engine.cpp
    Initializers.h
    Initializers.cpp
    Types.h
    Mesh.h
    Mesh.cpp
    bindings/imgui_impl_sdl.cpp
    bindings/imgui_impl_sdl.h
    bindings/imgui_impl_vulkan.cpp
    bindings/imgui_impl_vulkan.h
)

conan_target_link_libraries(svo-renderer)
target_include_directories(svo-renderer PRIVATE ${Vulkan_INCLUDE_DIR})
target_link_libraries(svo-renderer ${Vulkan_LIBRARY})