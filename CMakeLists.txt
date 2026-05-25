cmake_minimum_required(VERSION 3.21)
project(Anaros LANGUAGES CXX VERSION 0.1.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "" FORCE)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

find_package(yaml-cpp REQUIRED)

add_library(anaros INTERFACE)
add_library(Anaros::anaros ALIAS anaros)

target_include_directories(anaros INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_compile_features(anaros INTERFACE cxx_std_26)
target_link_libraries(anaros INTERFACE yaml-cpp::yaml-cpp)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 16)
    target_compile_options(anaros INTERFACE -freflection)
endif()

install(TARGETS anaros EXPORT AnarosTargets)
install(DIRECTORY src/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.h")
install(EXPORT AnarosTargets
        FILE AnarosTargets.cmake
        NAMESPACE Anaros::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Anaros)

configure_package_config_file(
    cmake/AnarosConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/AnarosConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Anaros)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/AnarosConfigVersion.cmake
    COMPATIBILITY SameMajorVersion)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/AnarosConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/AnarosConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Anaros)

include(CTest)
if(PROJECT_IS_TOP_LEVEL AND BUILD_TESTING)
    find_package(GTest REQUIRED)
    add_executable(anaros_test test/test.cpp)
    target_link_libraries(anaros_test PRIVATE Anaros::anaros GTest::gtest_main)
    add_test(NAME anaros_test COMMAND anaros_test)
endif()
