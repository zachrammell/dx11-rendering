# CMakeList.txt : CMake project for cs300-project, include source and define
# project specific logic here.
#
cmake_minimum_required(VERSION 3.8)

project("cs300-project")

# Add source to this project's executable.
add_executable(${PROJECT_NAME}
  "source/cs300-project.cpp"
  "source/cs300-project.h"
  "source/os_win32.h"
  "source/os_win32.cpp"
  "source/color.h"
  "source/render_dx11.h"
  "source/render_dx11.cpp"
 "source/callback-helper.h")

target_link_libraries(${PROJECT_NAME}
  user32
  d3d11
  dxgi
  d3dcompiler
)

target_compile_options(${PROJECT_NAME}
  PRIVATE "/std:c++17"
)

# TODO: Add tests and install targets if needed.
