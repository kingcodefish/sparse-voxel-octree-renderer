cmake_minimum_required(VERSION 3.0.0)
project(svo VERSION 0.1.0 LANGUAGES CXX CUDA)

find_package(GLUT REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)

include(CTest)
enable_testing()

add_executable(svo main.cu)
set_property(TARGET svo 
             PROPERTY CUDA_SEPARABLE_COMPILATION ON)

target_include_directories(svo PUBLIC ../cuda-by-example)
target_link_libraries(svo PUBLIC ${GLUT_LIBRARIES} ${OPENGL_LIBRARIES})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
