# CMakeList.txt : CMake project for cs350-project, include source and define
# project specific logic here.
#
cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project("impostor")

add_subdirectory("external")

# Add source to this project's executable.
add_executable(${PROJECT_NAME}
  "source/main.cpp"
  "source/platform/OS_Win32.h"
  "source/platform/OS_Win32.cpp"
  "source/graphics/Render_DX11.h"
  "source/graphics/Render_DX11.cpp"
  "source/CallbackHelper.h"
  "source/MathHelper.h"
  "source/MathHelper.cpp"
  "source/graphics/Shader.h"
  "source/graphics/Shader.cpp"
  "source/graphics/Mesh.h"
  "source/graphics/Mesh.cpp"
  "source/graphics/Texture.h"
  "source/graphics/Texture.cpp"
  "source/graphics/Cubemap.h"
  "source/graphics/Cubemap.cpp"
  "source/stb_image.h"
  "source/stb_image.c"
  "source/globals.h"
  "source/globals.cpp"
  "source/Image.h"
  "source/Image.cpp"
  "source/graphics/Framebuffer.h"
  "source/graphics/Framebuffer.cpp"
  "source/systems/Runner.h"
  "source/systems/ISystem.h"
  "source/systems/IModule.h"
  "source/NonCopyable.h" "source/Systems/Runner.cpp")

target_include_directories(${PROJECT_NAME}
  PRIVATE
  "${CMAKE_SOURCE_DIR}/source"
)

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
set(shader_flat ${CMAKE_SOURCE_DIR}/assets/shaders/flat.hlsl)
set(shader_flat_src ${CMAKE_SOURCE_DIR}/source/shaders/flat.hlsl)

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
add_custom_command(
  OUTPUT ${shader_flat}
  COMMAND ${CMAKE_C_COMPILER} /DCS350_HLSL_PREPROCESS /Zc:preprocessor /P /EP /Fi${shader_flat} ${shader_flat_src}
  DEPENDS ${shader_flat_src}
)

add_custom_target(
  shaders
  DEPENDS
  ${shader_deferred_geometry}
  ${shader_deferred_lighting}
  ${shader_debug_wireframe}
  ${shader_debug_line}
  ${shader_flat}
)

add_dependencies(
  ${PROJECT_NAME}
  shaders
)

# TODO: Add tests and install targets if needed.
