cmake_minimum_required(VERSION 3.15)
project(arborikit_cpp
        VERSION 0.1.0
        DESCRIPTION "General trees, unbalanced BSTs and red-black trees for C++ (header-only)"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ARBORIKIT_BUILD_TESTS "Build arborikit unit tests" ON)

# Header-only : une interface library, rien à compiler côté consommateur.
add_library(arborikit_cpp INTERFACE)
add_library(arborikit::cpp ALIAS arborikit_cpp)

target_include_directories(arborikit_cpp
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)

target_compile_features(arborikit_cpp INTERFACE cxx_std_11)

# Nom exporté : arborikit::cpp (et non arborikit::arborikit_cpp)
set_target_properties(arborikit_cpp PROPERTIES EXPORT_NAME cpp)

# ------------------------------- install -------------------------------
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS arborikit_cpp
    EXPORT arborikit_cppTargets
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT arborikit_cppTargets
    FILE arborikit_cppTargets.cmake
    NAMESPACE arborikit::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/arborikit_cpp)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/arborikit_cppConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/arborikit_cppConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/arborikit_cpp)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/arborikit_cppConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/arborikit_cppConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/arborikit_cppConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/arborikit_cpp)

# -------------------------------- tests ---------------------------------
if(ARBORIKIT_BUILD_TESTS)
    enable_testing()
    add_executable(arborikit_cpp_tests tests/test_arborikit.cpp)
    target_link_libraries(arborikit_cpp_tests PRIVATE arborikit_cpp)
    add_test(NAME arborikit_cpp_tests COMMAND arborikit_cpp_tests)
endif()
