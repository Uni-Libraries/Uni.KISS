# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText : 2022-2024 Uni-Libraries contributors

#
# CMake
#

# 3.21: PROJECT_IS_TOP_LEVEL and C_STANDARD 23
cmake_minimum_required(VERSION 3.21)

project(uni_kiss LANGUAGES C)


#
# Options
#

option(UNI_KISS_BUILD_TESTS "Build the Uni.KISS tests" ${PROJECT_IS_TOP_LEVEL})
option(UNI_KISS_INSTALL "Generate the Uni.KISS install rules" ${PROJECT_IS_TOP_LEVEL})


#
# Library
#

add_library(uni_kiss)
add_library(uni_kiss::uni_kiss ALIAS uni_kiss)

target_sources(uni_kiss PRIVATE
    src/uni_kiss.c
)

target_include_directories(uni_kiss PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

if(MSVC)
    set_target_properties(uni_kiss PROPERTIES C_STANDARD 17)
    target_compile_options(uni_kiss PRIVATE /W4)
else()
    set_target_properties(uni_kiss PROPERTIES C_STANDARD 23)
    target_compile_options(uni_kiss PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(uni_kiss PROPERTIES
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS OFF
    # the static library is meant to be linked into shared objects as well
    POSITION_INDEPENDENT_CODE ON
    # there are no export annotations; export everything when built as a DLL
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)


#
# Installation
#

if(UNI_KISS_INSTALL)
    install(
        TARGETS uni_kiss
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
    )
    install(FILES include/uni_kiss.h DESTINATION include)
endif()


#
# Tests
#

if(UNI_KISS_BUILD_TESTS)
    enable_language(CXX)
    enable_testing()
    add_subdirectory(src_tests)
endif()
