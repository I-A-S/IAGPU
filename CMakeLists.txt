cmake_minimum_required(VERSION 3.20)

project(IAGPU)
enable_testing()

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(IAGPU_IS_TOP_LEVEL ON)
else()
    set(IAGPU_IS_TOP_LEVEL OFF)
endif()

set(IAGPU_ROOT "${CMAKE_CURRENT_LIST_DIR}" CACHE INTERNAL "")

if(TARGET IACrux)
    message(STATUS "IAGPU: Using external IACrux provided by parent project")
else()
    message(STATUS "IAGPU: Building standalone, using local IACrux submodule")
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/IACrux/CMakeLists.txt")
        message(FATAL_ERROR "IACrux submodule missing! Run: git submodule update --init --recursive")
    endif()
    add_subdirectory(IACrux)
endif()

crux_setup_project()

option(IAGPU_ENABLE_BACKEND_VULKAN "Enable Vulkan Backend" ON)
option(IAGPU_ENABLE_BACKEND_WEBGPU "Enable WebGPU Backend" OFF)
option(IAGPU_ENABLE_PIPELINE_BAKER "Enable Pipeline Baker" ON)
option(IAGPU_DISABLE_GRAPHICS "Disable Graphics Support (Headless Mode)" OFF)

option(IAGPU_BUILD_AUX "Build Aux Helpers" ON)
option(IAGPU_BUILD_TESTS "Build unit tests" ${IAGPU_IS_TOP_LEVEL})

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/iagpu_config.hpp.in"
    "${CMAKE_BINARY_DIR}/generated/include/gpu/config.hpp"
)

include(cmake/find_deps.cmake)

add_subdirectory(src/)

if(IAGPU_BUILD_TESTS)
    add_subdirectory(tests)
endif()
