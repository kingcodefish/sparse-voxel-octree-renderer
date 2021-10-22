cmake_minimum_required(VERSION 3.0.0)
project(svo VERSION 1.0 LANGUAGES CXX CUDA)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

list(APPEND CMAKE_PREFIX_PATH 
    "libtorch")

find_package(GLUT REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(CNPY REQUIRED)
find_package(Torch REQUIRED)

find_library(CNPY_LIB cnpy)

include(CTest)
enable_testing()

add_executable(svo main.cu)
set_property(TARGET svo 
             PROPERTY CUDA_SEPARABLE_COMPILATION ON)

target_include_directories(svo PUBLIC ../cuda-by-example cub ${CNPY_INCLUDE_DIRS} ${TORCH_INCLUDE_DIRS})
target_link_libraries(svo PUBLIC ${CNPY_LIB} ${GLUT_LIBRARIES} ${OPENGL_LIBRARIES} ${TORCH_LIBRARIES} ${ZLIB_LIBRARIES})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
