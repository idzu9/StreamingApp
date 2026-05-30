# CMakeList.txt : CMake project for StreamingApp, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (StreamingApp "StreamingApp.cpp" "StreamingApp.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET StreamingApp PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
