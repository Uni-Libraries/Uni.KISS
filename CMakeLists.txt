# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText : 2022-2024 Uni-Libraries contributors

#
# CMake
#

cmake_minimum_required(VERSION 3.10)

project(uni_kiss)


#
# Library
#

add_library(uni_kiss)

target_sources(uni_kiss PRIVATE
    src/uni_kiss.c
)

target_include_directories(uni_kiss PUBLIC include)

if(MSVC)
    set_target_properties(uni_kiss PROPERTIES C_STANDARD 17)
else()
    set_target_properties(uni_kiss PROPERTIES C_STANDARD 23)
endif()



#
# Installation
#

install(
    TARGETS uni_kiss
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)



#
# Tests
#

if(PROJECT_IS_TOP_LEVEL)
    add_subdirectory(src_tests)
endif()
