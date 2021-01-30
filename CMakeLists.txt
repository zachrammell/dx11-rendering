# CMakeList.txt : CMake project for cs350-project, include source and define
# project specific logic here.
#
cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project("cs350-project")

# Build IMGUI
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/imgui")
set(IMGUI_INCLUDE_DIR "${IMGUI_DIR}" "${IMGUI_DIR}/examples")
file(GLOB IMGUI_SRC "${IMGUI_DIR}/*.cpp")
# Append IMGUI SRC with dx11 and win32
set(IMGUI_SRC
  ${IMGUI_SRC}
  ${IMGUI_DIR}/examples/imgui_impl_dx11.cpp
  ${IMGUI_DIR}/examples/imgui_impl_win32.cpp)
add_library(IMGUI ${IMGUI_SRC})
target_include_directories(IMGUI
  PUBLIC
  ${IMGUI_INCLUDE_DIR}
)
target_link_libraries(IMGUI
  PRIVATE
  user32
  d3d11
  dxgi
  d3dcompiler
)

# Add source to this project's executable.
add_executable(${PROJECT_NAME}
  "source/main.cpp"
  "source/os_win32.h"
  "source/os_win32.cpp"
  "source/render_dx11.h"
  "source/render_dx11.cpp"
  "source/callback_helper.h"
  "source/math_helper.h"
  "source/math_helper.cpp"
  "source/shader.h"
  "source/shader.cpp"
  "source/mesh.h"
  "source/mesh.cpp"
  "source/texture.h"
  "source/texture.cpp"
  "source/cubemap.h"
  "source/cubemap.cpp"
  "source/stb_image.h"
  "source/stb_image.c"
  "source/globals.h"
  "source/globals.cpp"
  "source/image.h"
  "source/image.cpp"
  "source/framebuffer.h"
  "source/framebuffer.cpp"
)

target_link_libraries(${PROJECT_NAME}
  user32
  d3d11
  dxgi
  d3dcompiler
  IMGUI
)

set(shader_sources
${CMAKE_SOURCE_DIR}/assets/shaders/deferred_geometry.hlsl
)

add_custom_target(shaders ALL
  DEPENDS ${shader_sources}
)

add_dependencies(${PROJECT_NAME} shaders)

function(preprocess_file)
  cmake_parse_arguments(
    ARG
    ""
    "FILE;OUTPUT"
    ""
    ${ARGN}
  )
  execute_process(COMMAND ${CMAKE_C_COMPILER} /DCS350_HLSL_PREPROCESS /Zc:preprocessor /P /EP /Fi${ARG_OUTPUT} ${ARG_FILE})
endfunction()

preprocess_file(FILE "${CMAKE_SOURCE_DIR}/source/shaders/deferred_geometry.hlsl" OUTPUT "${CMAKE_SOURCE_DIR}/assets/shaders/deferred_geometry.hlsl")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:${PROJECT_NAME}>/Assets)

# TODO: Add tests and install targets if needed.
