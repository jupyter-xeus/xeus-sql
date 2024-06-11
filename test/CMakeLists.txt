cmake_minimum_required(VERSION 3.1)

if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    project(xeus_sql-test)

    find_package(xeus REQUIRED CONFIG)
    set(XEUS_SQL_INCLUDE_DIR ${XEUS_SQL_INCLUDE_DIRS})
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

find_package(doctest REQUIRED)
find_package(Threads)

set(XEUS_SQL_TESTS
    main.cpp
    test_db.cpp
)

add_executable(test_xeus_sql ${XEUS_SQL_TESTS})

if (APPLE)
    set_target_properties(test_xeus_sql PROPERTIES
        MACOSX_RPATH ON
    )
else()
    set_target_properties(test_xeus_sql PROPERTIES
        BUILD_WITH_INSTALL_RPATH 1
        SKIP_BUILD_RPATH FALSE
    )
endif()

set_target_properties(test_xeus_sql PROPERTIES
    INSTALL_RPATH_USE_LINK_PATH TRUE
)

target_compile_features(test_xeus_sql PRIVATE cxx_std_17)

if (XSQL_BUILD_SHARED)
    set(XSQL_TEST_LINK_TARGET xeus-sql)
else()
    set(XSQL_TEST_LINK_TARGET xeus-sql-static)
endif()
include_directories(${XSQL_INCLUDE_DIRS})
target_link_libraries(test_xeus_sql PRIVATE ${XSQL_TEST_LINK_TARGET} xeus doctest::doctest ${CMAKE_THREAD_LIBS_INIT})
target_compile_definitions(test_xeus_sql PRIVATE XEUS_SQL_HOME="${XSQL_PREFIX}")
target_include_directories(test_xeus_sql PRIVATE ${XEUS_SQL_INCLUDE_DIR})

add_custom_target(
    xtest
    COMMAND test_xeus_sql
    DEPENDS test_xeus_sql)
