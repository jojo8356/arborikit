cmake_minimum_required(VERSION 3.15)
project(arborikit_c
        VERSION 0.1.0
        DESCRIPTION "General trees, unbalanced BSTs and red-black trees for C"
        LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(ARBORIKIT_BUILD_TESTS "Build arborikit unit tests" ON)

add_library(arborikit
    src/general_tree.c
    src/bst.c
    src/rbtree.c)
add_library(arborikit::arborikit ALIAS arborikit)

target_include_directories(arborikit
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(arborikit PRIVATE -Wall -Wextra -pedantic)
endif()

set_target_properties(arborikit PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    OUTPUT_NAME arborikit
    C_VISIBILITY_PRESET default)

# ------------------------------- install -------------------------------
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS arborikit
    EXPORT arborikitTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT arborikitTargets
    FILE arborikitTargets.cmake
    NAMESPACE arborikit::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/arborikit)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/arborikitConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/arborikitConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/arborikit)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/arborikitConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/arborikitConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/arborikitConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/arborikit)

# ------------------------------ pkg-config ------------------------------
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/arborikit.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/arborikit.pc
    @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/arborikit.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

# -------------------------------- tests ---------------------------------
if(ARBORIKIT_BUILD_TESTS)
    enable_testing()
    add_executable(arborikit_tests tests/test_arborikit.c)
    target_link_libraries(arborikit_tests PRIVATE arborikit)
    add_test(NAME arborikit_tests COMMAND arborikit_tests)
endif()
