# CMakeList.txt : CMake project for raylib_empty, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (raylib_empty "raylib_empty.cpp" "raylib_empty.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET raylib_empty PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
