# CMakeList.txt : CMake project for StreamingApp, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (StreamingApp "app/main.cpp"  "src/Server.cpp"  "src/WebrtcPipeline.cpp")

target_include_directories(StreamingApp PRIVATE 
    includes
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET StreamingApp PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
