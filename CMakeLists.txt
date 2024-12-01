cmake_minimum_required(VERSION 3.15)
project(prism_default_depth_stencil_view_patch LANGUAGES CXX)

add_library(${CMAKE_PROJECT_NAME} SHARED src/plugin.cpp)

add_subdirectory(external/PolyHook2)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE PolyHook_2)

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC include external/PolyHook2/PolyHook_2_0)

target_compile_features(${CMAKE_PROJECT_NAME} PUBLIC cxx_std_17)

set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE PRISM_DEFAULT_DEPTH_STENCIL_VIEW_PATCH_DLL_EXPORTS)