cmake_minimum_required(VERSION 3.1)

if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    project(xeus_soci-test)

    find_package(xeus REQUIRED CONFIG)
    set(XEUS_SOCI_INCLUDE_DIR ${XEUS_SOCI_INCLUDE_DIRS})
endif ()

if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "Setting tests build type to Release")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
else()
    message(STATUS "Tests build type is ${CMAKE_BUILD_TYPE}")
endif()


include(CheckCXXCompilerFlag)

string(TOUPPER "${CMAKE_BUILD_TYPE}" U_CMAKE_BUILD_TYPE)

if(CMAKE_CXX_COMPILER_ID MATCHES Clang OR CMAKE_CXX_COMPILER_ID MATCHES GNU OR CMAKE_CXX_COMPILER_ID MATCHES Intel)
    add_compile_options(-Wunused-parameter -Wextra -Wreorder -Wconversion -Wsign-conversion)

    CHECK_CXX_COMPILER_FLAG(-march=native HAS_MARCH_NATIVE)
    if (HAS_MARCH_NATIVE)
        add_compile_options(-march=native)
    endif()
endif()


if(CMAKE_CXX_COMPILER_ID MATCHES MSVC)
    add_compile_options(/EHsc /MP /bigobj)
    set(CMAKE_EXE_LINKER_FLAGS /MANIFEST:NO)
endif()


if(XSOCI_DOWNLOAD_GTEST OR GTEST_SRC_DIR)
    if(XSOCI_DOWNLOAD_GTEST)
        # Download and unpack googletest at configure time
        configure_file(downloadGTest.cmake.in googletest-download/CMakeLists.txt)
    else()
        # Copy local source of googletest at configure time
        configure_file(copyGTest.cmake.in googletest-download/CMakeLists.txt)
    endif()
    execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
                    RESULT_VARIABLE result
                    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
    if(result)
        message(FATAL_ERROR "CMake step for googletest failed: ${result}")
    endif()
    execute_process(COMMAND ${CMAKE_COMMAND} --build .
                    RESULT_VARIABLE result
                    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
    if(result)
        message(FATAL_ERROR "Build step for googletest failed: ${result}")
    endif()

    # Add googletest directly to our build. This defines
    # the gtest and gtest_main targets.
    add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
                     ${CMAKE_CURRENT_BINARY_DIR}/googletest-build EXCLUDE_FROM_ALL)

    set(GTEST_INCLUDE_DIRS "${gtest_SOURCE_DIR}/include")
    set(GTEST_BOTH_LIBRARIES gtest_main gtest)
else()
    find_package(GTest REQUIRED)
endif()

find_package(Threads)

include_directories(${GTEST_INCLUDE_DIRS} SYSTEM)


set(COMMON_BASE
    main.cpp
)

set(XEUS_SOCI_TESTS
    test_db.cpp
)

add_executable(test_xeus_soci  ${XEUS_SOCI_TESTS})

if(XSOCI_DOWNLOAD_GTEST OR GTEST_SRC_DIR)
    add_dependencies(test_xeus_soci gtest_main)
endif()

if (APPLE)
    set_target_properties(test_xeus_soci PROPERTIES
        MACOSX_RPATH ON
    )
else()
    set_target_properties(test_xeus_soci PROPERTIES
        BUILD_WITH_INSTALL_RPATH 1
        SKIP_BUILD_RPATH FALSE
    )
endif()

set_target_properties(test_xeus_soci PROPERTIES
    INSTALL_RPATH_USE_LINK_PATH TRUE
)

if (XSOCI_BUILD_SHARED)
    set(XSOCI_TEST_LINK_TARGET xeus-soci)
else()
    set(XSOCI_TEST_LINK_TARGET xeus-soci-static)
endif()
include_directories(${XSOCI_INCLUDE_DIRS})
target_link_libraries(test_xeus_soci PRIVATE ${XSOCI_TEST_LINK_TARGET} xeus ${GTEST_BOTH_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
target_compile_definitions(test_xeus_soci PRIVATE XEUS_SOCI_HOME="${XSOCI_PREFIX}")
target_include_directories(test_xeus_soci PRIVATE ${XEUS_SOCI_INCLUDE_DIR})

add_custom_target(
    xtest
    COMMAND test_xeus_soci
    DEPENDS test_xeus_soci)
