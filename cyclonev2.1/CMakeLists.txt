cmake_minimum_required(VERSION 3.8)
project(vehicle_side LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT TARGET CycloneDDS-CXX::ddscxx)
  find_package(CycloneDDS-CXX REQUIRED)
endif()

idlcxx_generate(TARGET controldata FILES ControlData.idl WARNINGS no-implicit-extensibility)

# include header files' path
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# add source files - command_publisher.cpp  entry.cpp  
set(SOURCES control_subscriber.cpp command_domain.cpp control_publisher.cpp ControlData.cpp wit_c_sdk.c shutdownsignal.cpp)

add_executable(vehicle_side ${SOURCES} "shutdownsignal.hpp" "serial.h")

# Link both executables to idl data type library and ddscxx.
target_link_libraries(vehicle_side PRIVATE CycloneDDS-CXX::ddscxx controldata)
target_link_libraries(vehicle_side PRIVATE pthread)
# link libraries about LogitecSteeringWheelLib
target_include_directories(vehicle_side PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Disable the static analyzer in GCC to avoid crashing the GNU C++ compiler
# on Azure Pipelines
if(DEFINED ENV{SYSTEM_TEAMFOUNDATIONSERVERURI})
  if(CMAKE_C_COMPILER_ID STREQUAL "GNU" AND ANALYZER STREQUAL "on")
    target_compile_options(vehicle_side PRIVATE -fno-analyzer)
  endif()
endif()

set_property(TARGET vehicle_side PROPERTY CXX_STANDARD 17)
