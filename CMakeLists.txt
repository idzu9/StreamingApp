cmake_minimum_required (VERSION 3.15)

if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("StreamingApp")

add_subdirectory ("StreamingApp")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GST REQUIRED gstreamer-1.0 gstreamer-webrtc-1.0 gstreamer-sdp-1.0 gstreamer-app-1.0 gstreamer-video-1.0)

find_package(Boost 1.65 REQUIRED COMPONENTS system filesystem json)
find_package(OpenCV REQUIRED)

target_include_directories(StreamingApp PRIVATE 
    ${GST_INCLUDE_DIRS} 
    ${Boost_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_directories(StreamingApp PRIVATE ${GST_LIBRARY_DIRS})

target_compile_definitions(StreamingApp PRIVATE GST_USE_UNSTABLE_API)

target_link_libraries(StreamingApp 
    ${GST_LIBRARIES} 
    Boost::system 
    Boost::filesystem 
    Boost::json 
    ${OpenCV_LIBS}
)

message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Boost Libraries: ${Boost_LIBRARIES}")
message(STATUS "GStreamer Libraries: ${GST_LIBRARIES}")
