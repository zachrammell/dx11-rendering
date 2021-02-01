# Build IMGUI
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/imgui")
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

set(CMAKE_BUILD_TYPE "Release")
option(BUILD_SHARED_LIBS OFF)
option(ASSIMP_INJECT_DEBUG_POSTFIX OFF)
option(ASSIMP_BUILD_TESTS OFF)
option(ASSIMP_INSTALL OFF)
add_subdirectory("assimp")
unset(CMAKE_BUILD_TYPE)
