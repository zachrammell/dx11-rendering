# CMakeList.txt : CMake project for cs350-project, include source and define
# project specific logic here.
#
cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project("cs350-project")

add_subdirectory("external")

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
  
  "source/geometry/vec.h"
  "source/geometry/ray.h"
  "source/geometry/sphere.h"
  "source/geometry/triangle.h"
  "source/geometry/box.h"
  "source/geometry/plane.h"
  "source/geometry/interval.h"
  "source/scene.h"
  "source/scene.cpp"
   "source/hierarchies/octree.h" "source/hierarchies/kdtree.h")

target_link_libraries(${PROJECT_NAME}
  user32
  d3d11
  dxgi
  d3dcompiler
  IMGUI
  assimp
)

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

# preprocess_file(FILE "${CMAKE_SOURCE_DIR}/source/shaders/deferred_geometry.hlsl" OUTPUT "${CMAKE_SOURCE_DIR}/assets/shaders/deferred_geometry.hlsl")

set(shader_deferred_geometry ${CMAKE_SOURCE_DIR}/assets/shaders/deferred_geometry.hlsl)
set(shader_deferred_geometry_src ${CMAKE_SOURCE_DIR}/source/shaders/deferred_geometry.hlsl)
set(shader_deferred_lighting ${CMAKE_SOURCE_DIR}/assets/shaders/deferred_lighting.hlsl)
set(shader_deferred_lighting_src ${CMAKE_SOURCE_DIR}/source/shaders/deferred_lighting.hlsl)
set(shader_debug_wireframe ${CMAKE_SOURCE_DIR}/assets/shaders/debug_wireframe.hlsl)
set(shader_debug_wireframe_src ${CMAKE_SOURCE_DIR}/source/shaders/debug_wireframe.hlsl)
set(shader_debug_line ${CMAKE_SOURCE_DIR}/assets/shaders/debug_line.hlsl)
set(shader_debug_line_src ${CMAKE_SOURCE_DIR}/source/shaders/debug_line.hlsl)

add_custom_command(
  OUTPUT ${shader_deferred_geometry}
  COMMAND ${CMAKE_C_COMPILER} /DCS350_HLSL_PREPROCESS /Zc:preprocessor /P /EP /Fi${shader_deferred_geometry} ${shader_deferred_geometry_src}
  DEPENDS ${shader_deferred_geometry_src}
)
add_custom_command(
  OUTPUT ${shader_deferred_lighting}
  COMMAND ${CMAKE_C_COMPILER} /DCS350_HLSL_PREPROCESS /Zc:preprocessor /P /EP /Fi${shader_deferred_lighting} ${shader_deferred_lighting_src}
  DEPENDS ${shader_deferred_lighting_src}
)
add_custom_command(
  OUTPUT ${shader_debug_wireframe}
  COMMAND ${CMAKE_C_COMPILER} /DCS350_HLSL_PREPROCESS /Zc:preprocessor /P /EP /Fi${shader_debug_wireframe} ${shader_debug_wireframe_src}
  DEPENDS ${shader_debug_wireframe_src}
)
add_custom_command(
  OUTPUT ${shader_debug_line}
  COMMAND ${CMAKE_C_COMPILER} /DCS350_HLSL_PREPROCESS /Zc:preprocessor /P /EP /Fi${shader_debug_line} ${shader_debug_line_src}
  DEPENDS ${shader_debug_line_src}
)

add_custom_target(
  shaders
  DEPENDS
  ${shader_deferred_geometry}
  ${shader_deferred_lighting}
  ${shader_debug_wireframe}
  ${shader_debug_line}
)

add_dependencies(
  ${PROJECT_NAME}
  shaders
)

# TODO: Add tests and install targets if needed.
