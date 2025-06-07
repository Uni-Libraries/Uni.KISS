# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText : 2022-2024 Uni-Libraries contributors

include(../cmake/cpm.cmake)
include(../cmake/cpm-catch.cmake)

# catch
list(APPEND CMAKE_MODULE_PATH "${Catch2_SOURCE_DIR}/extras")
include(Catch)

# test
add_executable(uni_kiss_test)
target_sources(uni_kiss_test PRIVATE uni_kiss_tests.cpp)
target_link_libraries(uni_kiss_test PRIVATE uni_kiss)
target_link_libraries(uni_kiss_test PRIVATE Catch2::Catch2WithMain)
catch_discover_tests(uni_kiss_test)
