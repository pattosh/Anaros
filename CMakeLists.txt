cmake_minimum_required(VERSION 3.21)
project(Anaros LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
  GIT_TAG yaml-cpp-0.9.0
)
FetchContent_MakeAvailable(yaml-cpp)

add_library(anaros INTERFACE)
add_library(Anaros::anaros ALIAS anaros)

target_compile_options(anaros INTERFACE -std=c++26 -freflection -fdiagnostics-color=always)

target_include_directories(anaros INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(anaros INTERFACE yaml-cpp::yaml-cpp)

add_executable(anaros_test test/test.cpp)
target_link_libraries(anaros_test PRIVATE Anaros::anaros)
